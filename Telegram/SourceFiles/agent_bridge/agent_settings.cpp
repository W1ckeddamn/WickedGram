/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#include "agent_bridge/agent_settings.h"

#include "settings.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QRandomGenerator>

namespace Agent {
namespace {

constexpr auto kDefaultPort = 40404;

} // namespace

Settings::Settings() {
	load();
}

QString Settings::ConfigPath() {
	return cWorkingDir() + u"tdata/wickedgram_agent.json"_q;
}

void Settings::setEnabled(bool enabled) {
	if (_enabled != enabled) {
		_enabled = enabled;
		save();
		_updates.fire({});
	}
}

void Settings::setPort(uint16 port) {
	if (_port != port && port > 0) {
		_port = port;
		save();
		_updates.fire({});
	}
}

void Settings::setToken(const QString &token) {
	if (_token != token && !token.trimmed().isEmpty()) {
		_token = token.trimmed();
		save();
		_updates.fire({});
	}
}

void Settings::regenerateToken() {
	const auto random = QRandomGenerator::global();
	QString token = u"wg_agent_"_q;
	for (auto i = 0; i < 4; ++i) {
		const auto val = random->generate();
		token += QString::number(val, 16).rightJustified(8, '0');
	}
	_token = token;
	save();
	_updates.fire({});
}

void Settings::ensureTokenGenerated() {
	if (_token.trimmed().isEmpty()) {
		regenerateToken();
	}
}

void Settings::load() {
	_enabled = true;
	_port = kDefaultPort;
	_token.clear();

	QFile file(ConfigPath());
	if (file.open(QIODevice::ReadOnly)) {
		const auto doc = QJsonDocument::fromJson(file.readAll());
		if (doc.isObject()) {
			const auto obj = doc.object();
			if (obj.contains("enabled")) {
				_enabled = obj["enabled"].toBool(true);
			}
			if (obj.contains("port")) {
				const auto portVal = obj["port"].toInt(kDefaultPort);
				if (portVal > 0 && portVal <= 65535) {
					_port = static_cast<uint16>(portVal);
				}
			}
			if (obj.contains("token")) {
				_token = obj["token"].toString().trimmed();
			}
		}
	}
	ensureTokenGenerated();
}

void Settings::save() {
	ensureTokenGenerated();

	const auto path = ConfigPath();
	const auto info = QFileInfo(path);
	QDir().mkpath(info.absolutePath());

	QJsonObject obj;
	obj["enabled"] = _enabled;
	obj["port"] = int(_port);
	obj["token"] = _token;

	QFile file(path);
	if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
	}
}

Settings &GlobalSettings() {
	static Settings instance;
	return instance;
}

} // namespace Agent
