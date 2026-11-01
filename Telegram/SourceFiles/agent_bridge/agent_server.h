/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#pragma once

#include "base/basic_types.h"
#include "base/flat_map.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QJsonObject>
#include <QtCore/QPointer>
#include <QtCore/QTimer>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>

namespace Agent {

struct HttpRequest {
	QString method;
	QString path;
	base::flat_map<QString, QString> query;
	base::flat_map<QString, QString> headers;
	QJsonObject body;
	bool isSSE = false;
};

struct HttpResponse {
	int status = 200;
	QString contentType = u"application/json; charset=utf-8"_q;
	QByteArray body;
	bool isSSE = false;

	static HttpResponse Json(int status, const QJsonObject &obj);
	static HttpResponse Error(int status, const QString &message);
	static HttpResponse SSE();
};

class Server final : public QObject {
public:
	using Handler = Fn<HttpResponse(const HttpRequest &)>;

	explicit Server(Handler handler, QObject *parent = nullptr);
	~Server();

	bool start(uint16 port, const QString &token);
	void stop();

	[[nodiscard]] bool isListening() const {
		return _server.isListening();
	}
	[[nodiscard]] uint16 port() const {
		return _server.serverPort();
	}

	void broadcastEvent(const QString &eventName, const QJsonObject &data);

private:
	struct ClientContext {
		QByteArray buffer;
		bool headersParsed = false;
		QString method;
		QString rawPath;
		base::flat_map<QString, QString> headers;
		int contentLength = 0;
	};

	void onNewConnection();
	void onSocketReadyRead(QTcpSocket *socket);
	void onSocketDisconnected(QTcpSocket *socket);

	void processRequest(QTcpSocket *socket, ClientContext &ctx);
	void sendResponse(QTcpSocket *socket, const HttpResponse &res);
	void sendSsePing();

	Handler _handler;
	QTcpServer _server;
	QString _token;

	base::flat_map<QTcpSocket*, ClientContext> _clients;
	std::vector<QPointer<QTcpSocket>> _sseSockets;
	QTimer _ssePingTimer;
};

} // namespace Agent
