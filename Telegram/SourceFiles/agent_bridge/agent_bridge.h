/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#pragma once

#include "agent_bridge/agent_server.h"
#include "agent_bridge/agent_settings.h"

#include <rpl/lifetime.h>
#include <memory>

class HistoryItem;
class History;
class PeerData;

namespace Main {
class Session;
} // namespace Main

namespace Agent {

class Bridge final {
public:
	static void Start();
	static void Finish();
	[[nodiscard]] static Bridge &Instance();

	Bridge();
	~Bridge();

	[[nodiscard]] bool isRunning() const;
	[[nodiscard]] uint16 port() const;
	[[nodiscard]] QString token() const;

	void setSession(Main::Session *session);

private:
	HttpResponse handleRequest(const HttpRequest &req);

	HttpResponse handleStatus(const HttpRequest &req);
	HttpResponse handleGetChats(const HttpRequest &req);
	HttpResponse handleGetChat(const HttpRequest &req, const QString &chatIdStr);
	HttpResponse handleGetMessages(const HttpRequest &req, const QString &chatIdStr);
	HttpResponse handleSendMessage(const HttpRequest &req, const QString &chatIdStr);
	HttpResponse handleMarkRead(const HttpRequest &req, const QString &chatIdStr);
	HttpResponse handleSendTyping(const HttpRequest &req, const QString &chatIdStr);
	HttpResponse handleOpenChat(const HttpRequest &req);

	void onNewItemAdded(not_null<HistoryItem*> item);
	void updateServerState();

	std::unique_ptr<Server> _server;
	Main::Session *_session = nullptr;

	rpl::lifetime _sessionLifetime;
	rpl::lifetime _lifetime;
};

} // namespace Agent
