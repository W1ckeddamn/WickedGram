/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#include "agent_bridge/agent_bridge.h"

#include "core/application.h"
#include "main/main_domain.h"
#include "main/main_session.h"
#include "main/main_account.h"
#include "data/data_session.h"
#include "data/data_peer.h"
#include "data/data_user.h"
#include "data/data_channel.h"
#include "data/data_chat.h"
#include "data/data_histories.h"
#include "history/history.h"
#include "history/history_item.h"
#include "history/view/history_view_element.h"
#include "apiwrap.h"
#include "api/api_common.h"
#include "api/api_send_progress.h"
#include "dialogs/dialogs_main_list.h"
#include "dialogs/dialogs_indexed_list.h"
#include "dialogs/dialogs_row.h"
#include "window/window_session_controller.h"
#include "window/window_controller.h"

#include "wicked/wicked_regdate.h"
#include "wicked/wicked_realname.h"
#include "wicked/wicked_backup.h"
#include "wicked/wicked_chat_exporter.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QRegularExpression>

namespace Agent {
namespace {

QJsonObject ItemToJson(not_null<HistoryItem*> item) {
	QJsonObject obj;
	obj["id"] = qint64(item->id.bare);
	obj["date"] = qint64(item->date());
	obj["out"] = item->out();
	obj["text"] = item->originalText().text;
	obj["unread"] = item->isUnread();

	if (const auto from = item->from()) {
		obj["from_id"] = QString::number(from->id.value);
		obj["from_name"] = from->name();
		if (const auto user = from->asUser()) {
			obj["from_username"] = user->username();
		}
	}
	if (const auto peer = item->history()->peer) {
		obj["chat_id"] = QString::number(peer->id.value);
		obj["chat_title"] = peer->name();
	}
	if (const auto replyTo = item->replyToId(); replyTo.bare != 0) {
		obj["reply_to_id"] = qint64(replyTo.bare);
	}
	obj["has_media"] = (item->media() != nullptr);
	return obj;
}

QJsonObject PeerToJson(not_null<PeerData*> peer, History *history) {
	QJsonObject obj;
	obj["id"] = QString::number(peer->id.value);
	obj["title"] = peer->name();
	obj["username"] = peer->username();
	if (peer->isUser()) {
		obj["type"] = "user";
		if (const auto user = peer->asUser()) {
			obj["first_name"] = user->firstName;
			obj["last_name"] = user->lastName;
			obj["phone"] = user->phone();
			obj["is_bot"] = user->isBot();
			const auto rawUserId = peer->id.value & PeerId::kChatTypeMask;
			obj["registration_date"] = Wicked::RegDate::FormatRegistrationDate(rawUserId);
			const auto origName = Wicked::RealNameCache::Instance().getOriginalName(rawUserId);
			if (!origName.isEmpty()) {
				obj["original_name"] = origName;
			}
		}
	} else if (peer->isChannel()) {
		obj["type"] = (peer->asChannel() && peer->asChannel()->isBroadcast())
			? "channel"
			: "supergroup";
	} else if (peer->isChat()) {
		obj["type"] = "group";
	} else {
		obj["type"] = "chat";
	}

	if (history) {
		obj["unread_count"] = history->unreadCount();
		if (const auto last = history->lastMessage()) {
			QJsonObject lastObj;
			lastObj["id"] = qint64(last->id.bare);
			lastObj["date"] = qint64(last->date());
			lastObj["text"] = last->originalText().text;
			lastObj["out"] = last->out();
			if (const auto from = last->from()) {
				lastObj["from_name"] = from->name();
			}
			obj["last_message"] = lastObj;
		}
	}
	return obj;
}

} // namespace

void Bridge::Start() {
	Instance().updateServerState();
}

void Bridge::Finish() {
	Instance()._server->stop();
}

Bridge &Bridge::Instance() {
	static Bridge instance;
	return instance;
}

Bridge::Bridge() {
	_server = std::make_unique<Server>([=](const HttpRequest &req) {
		return handleRequest(req);
	});

	// Listen to active session changes across domain
	Core::App().domain().activeSessionChanges(
	) | rpl::start_with_next([=](Main::Session *session) {
		setSession(session);
	}, _lifetime);

	// Listen to setting changes (port, token, enabled)
	GlobalSettings().updates(
	) | rpl::start_with_next([=] {
		updateServerState();
	}, _lifetime);

	updateServerState();
}

Bridge::~Bridge() {
	if (_server) {
		_server->stop();
	}
}

bool Bridge::isRunning() const {
	return _server && _server->isListening();
}

uint16 Bridge::port() const {
	return _server ? _server->port() : 0;
}

QString Bridge::token() const {
	return GlobalSettings().token();
}

void Bridge::updateServerState() {
	const auto &settings = GlobalSettings();
	if (!settings.enabled()) {
		_server->stop();
		return;
	}

	_server->start(settings.port(), settings.token());
}

void Bridge::setSession(Main::Session *session) {
	_session = session;
	_sessionLifetime.destroy();

	if (_session) {
		_session->data().newItemAdded(
		) | rpl::start_with_next([=](not_null<HistoryItem*> item) {
			onNewItemAdded(item);
		}, _sessionLifetime);
	}
}

void Bridge::onNewItemAdded(not_null<HistoryItem*> item) {
	if (!_server || !_server->isListening()) {
		return;
	}

	QJsonObject data = ItemToJson(item);
	data["event"] = "new_message";
	_server->broadcastEvent(u"message"_q, data);
}

HttpResponse Bridge::handleRequest(const HttpRequest &req) {
	if (req.path == u"/api/status"_q) {
		return handleStatus(req);
	} else if (req.path == u"/api/chats"_q) {
		return handleGetChats(req);
	} else if (req.path == u"/api/events"_q) {
		return HttpResponse::SSE();
	} else if (req.path == u"/api/ui/open_chat"_q) {
		if (req.method != u"POST"_q) {
			return HttpResponse::Error(405, u"Method Not Allowed: use POST"_q);
		}
		return handleOpenChat(req);
	} else if (req.path == u"/api/user/regdate"_q) {
		if (req.method != u"GET"_q) {
			return HttpResponse::Error(405, u"Method Not Allowed: use GET"_q);
		}
		const auto userIt = req.query.find(u"user_id"_q);
		if (userIt == req.query.end()) {
			return HttpResponse::Error(400, u"Missing user_id query parameter"_q);
		}
		bool ok = false;
		const auto uid = userIt->second.toULongLong(&ok);
		if (!ok || uid == 0) {
			return HttpResponse::Error(400, u"Invalid user_id"_q);
		}
		const auto rawUserId = uid & PeerId::kChatTypeMask;
		const auto ts = Wicked::RegDate::EstimateTimestamp(rawUserId);
		const auto formatted = Wicked::RegDate::FormatRegistrationDate(rawUserId);
		QJsonObject res;
		res["status"] = "ok";
		res["user_id"] = QString::number(uid);
		res["timestamp"] = qint64(ts);
		res["registration_date"] = formatted;
		return HttpResponse::Json(200, res);
	} else if (req.path == u"/api/settings/backup/export"_q) {
		if (req.method != u"POST"_q) {
			return HttpResponse::Error(405, u"Method Not Allowed: use POST"_q);
		}
		const auto path = Wicked::BackupManager::ExportBackup();
		if (path.isEmpty()) {
			return HttpResponse::Error(500, u"Failed to export backup"_q);
		}
		QJsonObject res;
		res["status"] = "ok";
		res["backup_path"] = path;
		return HttpResponse::Json(200, res);
	} else if (req.path == u"/api/settings/backup/import"_q) {
		if (req.method != u"POST"_q) {
			return HttpResponse::Error(405, u"Method Not Allowed: use POST"_q);
		}
		const auto doc = QJsonDocument::fromJson(req.body);
		QString path;
		if (doc.isObject()) {
			path = doc.object().value("file_path").toString();
		}
		if (path.isEmpty()) {
			QDir dir(Wicked::BackupManager::DefaultBackupDirectory());
			const auto list = dir.entryInfoList(QStringList() << u"*.json"_q, QDir::Files, QDir::Time);
			if (!list.isEmpty()) {
				path = list.first().absoluteFilePath();
			}
		}
		if (path.isEmpty() || !Wicked::BackupManager::ImportBackup(path)) {
			return HttpResponse::Error(400, u"Failed to import backup from: " + path);
		}
		QJsonObject res;
		res["status"] = "ok";
		res["imported_from"] = path;
		return HttpResponse::Json(200, res);
	}

	// Route /api/chats/{chat_id}(/(messages|read|typing))?
	static const auto chatRoute = QRegularExpression(
		u"^/api/chats/([^/]+)(?:/(messages|read|typing))?$"_q);
	const auto match = chatRoute.match(req.path);
	if (match.hasMatch()) {
		const auto chatIdStr = match.captured(1);
		const auto action = match.captured(2);

		if (action.isEmpty()) {
			if (req.method == u"GET"_q) {
				return handleGetChat(req, chatIdStr);
			}
			return HttpResponse::Error(405, u"Method Not Allowed: use GET"_q);
		} else if (action == u"messages"_q) {
			if (req.method == u"GET"_q) {
				return handleGetMessages(req, chatIdStr);
			} else if (req.method == u"POST"_q) {
				return handleSendMessage(req, chatIdStr);
			}
			return HttpResponse::Error(405, u"Method Not Allowed"_q);
		} else if (action == u"read"_q) {
			if (req.method == u"POST"_q) {
				return handleMarkRead(req, chatIdStr);
			}
			return HttpResponse::Error(405, u"Method Not Allowed: use POST"_q);
		} else if (action == u"typing"_q) {
			if (req.method == u"POST"_q) {
				return handleSendTyping(req, chatIdStr);
			}
			return HttpResponse::Error(405, u"Method Not Allowed: use POST"_q);
		}
	}

	return HttpResponse::Error(404, u"Endpoint Not Found: " + req.path);
}

HttpResponse Bridge::handleStatus(const HttpRequest &req) {
	QJsonObject res;
	res["status"] = "ok";
	res["client"] = "WickedGram Desktop";
	res["bridge_listening"] = isRunning();
	res["bridge_port"] = int(port());

	const auto session = _session ? _session : Core::App().activeAccount().maybeSession();
	if (session && session->user()) {
		res["logged_in"] = true;
		const auto user = session->user();
		QJsonObject userObj;
		userObj["id"] = QString::number(user->id.value);
		userObj["name"] = user->name();
		userObj["username"] = user->username();
		userObj["phone"] = user->phone();
		res["user"] = userObj;
	} else {
		res["logged_in"] = false;
	}

	return HttpResponse::Json(200, res);
}

HttpResponse Bridge::handleGetChats(const HttpRequest &req) {
	const auto session = _session ? _session : Core::App().activeAccount().maybeSession();
	if (!session) {
		return HttpResponse::Error(503, u"Client not logged in"_q);
	}

	int limit = 50;
	const auto limitIt = req.query.find(u"limit"_q);
	if (limitIt != req.query.end()) {
		limit = std::clamp(limitIt->second.toInt(), 1, 200);
	}

	const auto queryIt = req.query.find(u"query"_q);
	const auto searchQuery = (queryIt != req.query.end()) ? queryIt->second.toLower() : QString();

	QJsonArray chatsArray;
	const auto list = session->data().chatsList();
	if (list && list->indexed()) {
		for (const auto &row : list->indexed()->all()) {
			const auto key = row->key();
			const auto history = key.history();
			const auto peer = history ? history->peer.get() : key.peer().get();
			if (!peer) {
				continue;
			}

			if (!searchQuery.isEmpty()) {
				const auto titleMatches = peer->name().toLower().contains(searchQuery);
				const auto usernameMatches = peer->username().toLower().contains(searchQuery);
				if (!titleMatches && !usernameMatches) {
					continue;
				}
			}

			chatsArray.append(PeerToJson(peer, history));
			if (chatsArray.size() >= limit) {
				break;
			}
		}
	}

	QJsonObject res;
	res["status"] = "ok";
	res["count"] = chatsArray.size();
	res["chats"] = chatsArray;
	return HttpResponse::Json(200, res);
}

HttpResponse Bridge::handleGetChat(const HttpRequest &req, const QString &chatIdStr) {
	const auto session = _session ? _session : Core::App().activeAccount().maybeSession();
	if (!session) {
		return HttpResponse::Error(503, u"Client not logged in"_q);
	}

	bool ok = false;
	const auto rawId = chatIdStr.toULongLong(&ok);
	if (!ok || rawId == 0) {
		return HttpResponse::Error(400, u"Invalid chat_id format"_q);
	}

	const auto peer = session->data().peerLoaded(PeerId(rawId));
	if (!peer) {
		return HttpResponse::Error(404, u"Chat not found"_q);
	}

	const auto history = session->data().historyLoaded(peer);
	QJsonObject res;
	res["status"] = "ok";
	res["chat"] = PeerToJson(peer, history);
	return HttpResponse::Json(200, res);
}

HttpResponse Bridge::handleGetMessages(const HttpRequest &req, const QString &chatIdStr) {
	const auto session = _session ? _session : Core::App().activeAccount().maybeSession();
	if (!session) {
		return HttpResponse::Error(503, u"Client not logged in"_q);
	}

	bool ok = false;
	const auto rawId = chatIdStr.toULongLong(&ok);
	if (!ok || rawId == 0) {
		return HttpResponse::Error(400, u"Invalid chat_id format"_q);
	}

	const auto history = session->data().historyLoaded(PeerId(rawId));
	if (!history) {
		return HttpResponse::Error(404, u"History not loaded for chat"_q);
	}

	int limit = 30;
	const auto limitIt = req.query.find(u"limit"_q);
	if (limitIt != req.query.end()) {
		limit = std::clamp(limitIt->second.toInt(), 1, 100);
	}

	std::vector<not_null<HistoryItem*>> allItems;
	for (const auto &block : history->blocks) {
		for (const auto &view : block->messages) {
			allItems.push_back(view->data());
		}
	}

	QJsonArray messagesArray;
	const auto startIdx = (int(allItems.size()) > limit)
		? (int(allItems.size()) - limit)
		: 0;
	for (auto i = startIdx; i < int(allItems.size()); ++i) {
		messagesArray.append(ItemToJson(allItems[i]));
	}

	QJsonObject res;
	res["status"] = "ok";
	res["chat_id"] = chatIdStr;
	res["count"] = messagesArray.size();
	res["messages"] = messagesArray;
	return HttpResponse::Json(200, res);
}

HttpResponse Bridge::handleSendMessage(const HttpRequest &req, const QString &chatIdStr) {
	const auto session = _session ? _session : Core::App().activeAccount().maybeSession();
	if (!session) {
		return HttpResponse::Error(503, u"Client not logged in"_q);
	}

	const auto text = req.body["text"].toString().trimmed();
	if (text.isEmpty()) {
		return HttpResponse::Error(400, u"Missing or empty 'text' in body"_q);
	}

	bool ok = false;
	const auto rawId = chatIdStr.toULongLong(&ok);
	if (!ok || rawId == 0) {
		return HttpResponse::Error(400, u"Invalid chat_id format"_q);
	}

	const auto replyToVal = req.body["reply_to"].toVariant().toLongLong();

	const auto history = session->data().history(PeerId(rawId));
	auto message = Api::MessageToSend(Api::SendAction(history));
	message.textWithTags = { text, {} };
	if (replyToVal != 0) {
		message.action.replyTo.messageId = { history->peer->id, MsgId(replyToVal) };
	}
	message.action.clearDraft = false;
	session->api().sendMessage(std::move(message));

	QJsonObject res;
	res["status"] = "ok";
	res["chat_id"] = chatIdStr;
	res["sent"] = true;
	return HttpResponse::Json(200, res);
}

HttpResponse Bridge::handleMarkRead(const HttpRequest &req, const QString &chatIdStr) {
	const auto session = _session ? _session : Core::App().activeAccount().maybeSession();
	if (!session) {
		return HttpResponse::Error(503, u"Client not logged in"_q);
	}

	bool ok = false;
	const auto rawId = chatIdStr.toULongLong(&ok);
	if (!ok || rawId == 0) {
		return HttpResponse::Error(400, u"Invalid chat_id format"_q);
	}

	const auto history = session->data().history(PeerId(rawId));
	session->data().histories().readInbox(history);

	QJsonObject res;
	res["status"] = "ok";
	res["chat_id"] = chatIdStr;
	res["marked_read"] = true;
	return HttpResponse::Json(200, res);
}

HttpResponse Bridge::handleSendTyping(const HttpRequest &req, const QString &chatIdStr) {
	const auto session = _session ? _session : Core::App().activeAccount().maybeSession();
	if (!session) {
		return HttpResponse::Error(503, u"Client not logged in"_q);
	}

	bool ok = false;
	const auto rawId = chatIdStr.toULongLong(&ok);
	if (!ok || rawId == 0) {
		return HttpResponse::Error(400, u"Invalid chat_id format"_q);
	}

	const auto history = session->data().history(PeerId(rawId));
	session->sendProgressManager().update(history, Api::SendProgressType::Typing);

	QJsonObject res;
	res["status"] = "ok";
	res["chat_id"] = chatIdStr;
	res["action"] = "typing";
	return HttpResponse::Json(200, res);
}

HttpResponse Bridge::handleOpenChat(const HttpRequest &req) {
	const auto session = _session ? _session : Core::App().activeAccount().maybeSession();
	if (!session) {
		return HttpResponse::Error(503, u"Client not logged in"_q);
	}

	const auto chatIdStr = req.body["chat_id"].toVariant().toString().trimmed();
	bool ok = false;
	const auto rawId = chatIdStr.toULongLong(&ok);
	if (!ok || rawId == 0) {
		return HttpResponse::Error(400, u"Missing or invalid 'chat_id' in body"_q);
	}

	if (const auto window = Core::App().activeWindow()) {
		if (const auto controller = window->sessionController()) {
			controller->showPeerHistory(PeerId(rawId));
			window->activateWindow();
		}
	}

	QJsonObject res;
	res["status"] = "ok";
	res["chat_id"] = chatIdStr;
	res["opened"] = true;
	return HttpResponse::Json(200, res);
}

} // namespace Agent
