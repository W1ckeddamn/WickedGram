/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#include "agent_bridge/agent_server.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QtNetwork/QHostAddress>

namespace Agent {
namespace {

QString HttpStatusText(int status) {
	switch (status) {
	case 200: return u"OK"_q;
	case 201: return u"Created"_q;
	case 204: return u"No Content"_q;
	case 400: return u"Bad Request"_q;
	case 401: return u"Unauthorized"_q;
	case 403: return u"Forbidden"_q;
	case 404: return u"Not Found"_q;
	case 405: return u"Method Not Allowed"_q;
	case 500: return u"Internal Server Error"_q;
	case 503: return u"Service Unavailable"_q;
	default: return u"Unknown"_q;
	}
}

} // namespace

HttpResponse HttpResponse::Json(int status, const QJsonObject &obj) {
	HttpResponse res;
	res.status = status;
	res.contentType = u"application/json; charset=utf-8"_q;
	res.body = QJsonDocument(obj).toJson(QJsonDocument::Compact);
	return res;
}

HttpResponse HttpResponse::Error(int status, const QString &message) {
	QJsonObject obj;
	obj["status"] = "error";
	obj["code"] = status;
	obj["error"] = message;
	return Json(status, obj);
}

HttpResponse HttpResponse::SSE() {
	HttpResponse res;
	res.status = 200;
	res.isSSE = true;
	return res;
}

Server::Server(Handler handler, QObject *parent)
: QObject(parent)
, _handler(std::move(handler)) {
	connect(&_server, &QTcpServer::newConnection, this, &Server::onNewConnection);
	connect(&_ssePingTimer, &QTimer::timeout, this, &Server::sendSsePing);
	_ssePingTimer.setInterval(15000);
}

Server::~Server() {
	stop();
}

bool Server::start(uint16 port, const QString &token) {
	stop();
	_token = token.trimmed();

	const auto success = _server.listen(QHostAddress::LocalHost, port);
	if (success) {
		_ssePingTimer.start();
	}
	return success;
}

void Server::stop() {
	_ssePingTimer.stop();
	for (const auto &socket : _sseSockets) {
		if (socket) {
			socket->disconnectFromHost();
		}
	}
	_sseSockets.clear();

	for (const auto &[socket, ctx] : _clients) {
		if (socket) {
			socket->disconnectFromHost();
		}
	}
	_clients.clear();

	if (_server.isListening()) {
		_server.close();
	}
}

void Server::onNewConnection() {
	while (_server.hasPendingConnections()) {
		auto socket = _server.nextPendingConnection();
		if (!socket) {
			continue;
		}

		// Security: verify local loopback origin
		const auto peer = socket->peerAddress();
		const auto isLoopback = (peer == QHostAddress::LocalHost)
			|| (peer == QHostAddress::LocalHostIPv6)
			|| (peer == QHostAddress(u"127.0.0.1"_q));

		if (!isLoopback) {
			socket->abort();
			continue;
		}

		_clients.emplace(socket, ClientContext());

		connect(socket, &QTcpSocket::readyRead, this, [=] {
			onSocketReadyRead(socket);
		});
		connect(socket, &QTcpSocket::disconnected, this, [=] {
			onSocketDisconnected(socket);
		});
	}
}

void Server::onSocketDisconnected(QTcpSocket *socket) {
	_clients.erase(socket);

	// Remove from SSE sockets
	_sseSockets.erase(
		std::remove_if(
			_sseSockets.begin(),
			_sseSockets.end(),
			[=](const QPointer<QTcpSocket> &p) {
				return !p || p.data() == socket;
			}),
		_sseSockets.end());

	socket->deleteLater();
}

void Server::onSocketReadyRead(QTcpSocket *socket) {
	const auto it = _clients.find(socket);
	if (it == _clients.end()) {
		return;
	}
	auto &ctx = it->second;
	ctx.buffer.append(socket->readAll());

	if (!ctx.headersParsed) {
		const auto headerEnd = ctx.buffer.indexOf("\r\n\r\n");
		if (headerEnd == -1) {
			return; // wait for full headers
		}

		const auto headerData = ctx.buffer.left(headerEnd);
		ctx.buffer.remove(0, headerEnd + 4); // retain remaining body bytes

		const auto lines = QString::fromUtf8(headerData).split(u"\r\n"_q);
		if (lines.isEmpty()) {
			sendResponse(socket, HttpResponse::Error(400, u"Malformed HTTP request"_q));
			return;
		}

		// Parse request line: METHOD PATH HTTP/1.x
		const auto reqLineParts = lines[0].split(' ', Qt::SkipEmptyParts);
		if (reqLineParts.size() < 2) {
			sendResponse(socket, HttpResponse::Error(400, u"Invalid HTTP request line"_q));
			return;
		}
		ctx.method = reqLineParts[0].toUpper();
		ctx.rawPath = reqLineParts[1];

		// Parse headers
		for (auto i = 1; i < lines.size(); ++i) {
			const auto colonIdx = lines[i].indexOf(':');
			if (colonIdx != -1) {
				const auto key = lines[i].left(colonIdx).trimmed().toLower();
				const auto val = lines[i].mid(colonIdx + 1).trimmed();
				ctx.headers[key] = val;
			}
		}

		const auto clIt = ctx.headers.find(u"content-length"_q);
		if (clIt != ctx.headers.end()) {
			ctx.contentLength = clIt->second.toInt();
		}
		ctx.headersParsed = true;
	}

	// Wait for body if Content-Length was specified
	if (ctx.contentLength > 0 && ctx.buffer.size() < ctx.contentLength) {
		return;
	}

	processRequest(socket, ctx);
}

void Server::processRequest(QTcpSocket *socket, ClientContext &ctx) {
	// CORS Preflight request
	if (ctx.method == u"OPTIONS"_q) {
		HttpResponse res;
		res.status = 200;
		sendResponse(socket, res);
		return;
	}

	// Parse URL path and query string
	const auto url = QUrl(ctx.rawPath);
	const auto path = url.path();
	const auto urlQuery = QUrlQuery(url.query());

	base::flat_map<QString, QString> queryParams;
	for (const auto &[k, v] : urlQuery.queryItems()) {
		queryParams[k] = v;
	}

	// Authentication check
	bool authenticated = false;
	const auto authIt = ctx.headers.find(u"authorization"_q);
	if (authIt != ctx.headers.end()) {
		const auto val = authIt->second;
		if (val.startsWith(u"Bearer "_q, Qt::CaseInsensitive)) {
			const auto token = val.mid(7).trimmed();
			if (token == _token && !_token.isEmpty()) {
				authenticated = true;
			}
		}
	}
	if (!authenticated) {
		const auto tokenParamIt = queryParams.find(u"token"_q);
		if (tokenParamIt != queryParams.end() && tokenParamIt->second == _token && !_token.isEmpty()) {
			authenticated = true;
		}
	}

	if (!authenticated) {
		sendResponse(socket, HttpResponse::Error(401, u"Unauthorized: Invalid or missing Bearer token"_q));
		return;
	}

	// Parse JSON body if present
	QJsonObject bodyObj;
	if (!ctx.buffer.isEmpty()) {
		const auto doc = QJsonDocument::fromJson(ctx.buffer.left(ctx.contentLength > 0 ? ctx.contentLength : ctx.buffer.size()));
		if (doc.isObject()) {
			bodyObj = doc.object();
		}
	}

	HttpRequest req;
	req.method = ctx.method;
	req.path = path;
	req.query = std::move(queryParams);
	req.headers = std::move(ctx.headers);
	req.body = std::move(bodyObj);

	// Dispatch to handler
	const auto res = _handler ? _handler(req) : HttpResponse::Error(404, u"No handler"_q);

	if (res.isSSE) {
		// Handle Server-Sent Events stream
		QByteArray sseHeader =
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/event-stream\r\n"
			"Cache-Control: no-cache\r\n"
			"Connection: keep-alive\r\n"
			"Access-Control-Allow-Origin: *\r\n"
			"Access-Control-Allow-Headers: Authorization, Content-Type\r\n"
			"\r\n"
			": connected\n\n";

		socket->write(sseHeader);
		socket->flush();

		_clients.erase(socket);
		_sseSockets.push_back(socket);
	} else {
		sendResponse(socket, res);
	}
}

void Server::sendResponse(QTcpSocket *socket, const HttpResponse &res) {
	const auto statusText = HttpStatusText(res.status);
	QByteArray header =
		"HTTP/1.1 " + QByteArray::number(res.status) + " " + statusText.toUtf8() + "\r\n"
		"Content-Type: " + res.contentType.toUtf8() + "\r\n"
		"Content-Length: " + QByteArray::number(res.body.size()) + "\r\n"
		"Access-Control-Allow-Origin: *\r\n"
		"Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
		"Access-Control-Allow-Headers: Authorization, Content-Type\r\n"
		"Connection: close\r\n"
		"\r\n";

	socket->write(header);
	if (!res.body.isEmpty()) {
		socket->write(res.body);
	}
	socket->flush();
	socket->disconnectFromHost();
}

void Server::broadcastEvent(const QString &eventName, const QJsonObject &data) {
	if (_sseSockets.empty()) {
		return;
	}

	const auto jsonBytes = QJsonDocument(data).toJson(QJsonDocument::Compact);
	const auto eventData =
		"event: " + eventName.toUtf8() + "\n"
		"data: " + jsonBytes + "\n\n";

	for (auto it = _sseSockets.begin(); it != _sseSockets.end();) {
		if (*it && (*it)->state() == QAbstractSocket::ConnectedState) {
			(*it)->write(eventData);
			(*it)->flush();
			++it;
		} else {
			it = _sseSockets.erase(it);
		}
	}
}

void Server::sendSsePing() {
	if (_sseSockets.empty()) {
		return;
	}
	const auto ping = ": ping\n\n";
	for (auto it = _sseSockets.begin(); it != _sseSockets.end();) {
		if (*it && (*it)->state() == QAbstractSocket::ConnectedState) {
			(*it)->write(ping);
			(*it)->flush();
			++it;
		} else {
			it = _sseSockets.erase(it);
		}
	}
}

} // namespace Agent
