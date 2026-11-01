/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#pragma once

#include <QtCore/QString>
#include <rpl/event_stream.h>
#include <rpl/producer.h>

namespace Agent {

class Settings final {
public:
	Settings();

	[[nodiscard]] bool enabled() const {
		return _enabled;
	}
	[[nodiscard]] uint16 port() const {
		return _port;
	}
	[[nodiscard]] QString token() const {
		return _token;
	}

	void setEnabled(bool enabled);
	void setPort(uint16 port);
	void setToken(const QString &token);
	void regenerateToken();

	void load();
	void save();

	[[nodiscard]] rpl::producer<> updates() const {
		return _updates.events();
	}

	[[nodiscard]] static QString ConfigPath();

private:
	void ensureTokenGenerated();

	bool _enabled = true;
	uint16 _port = 40404;
	QString _token;

	rpl::event_stream<> _updates;
};

[[nodiscard]] Settings &GlobalSettings();

} // namespace Agent
