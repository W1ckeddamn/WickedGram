/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#include "wicked/wicked_backup.h"

#include "wicked/wicked_config.h"
#include "storage/localstorage.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDateTime>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

namespace Wicked {

QString BackupManager::DefaultBackupDirectory() {
	const auto dir = cWorkingDir() + u"tdata/backups/"_q;
	QDir().mkpath(dir);
	return dir;
}

QString BackupManager::ExportBackup(const QString &targetFilePath) {
	auto dest = targetFilePath;
	if (dest.isEmpty()) {
		const auto timestamp = QDateTime::currentDateTime().toString(u"yyyyMMdd_HHmmss"_q);
		dest = DefaultBackupDirectory() + u"wickedgram_settings_"_q + timestamp + u".json"_q;
	}

	QJsonObject configObj;
	configObj.insert("bypassContentProtection", Config::BypassContentProtection());
	configObj.insert("ghostMode", Config::GhostMode());
	configObj.insert("stealthTyping", Config::StealthTyping());
	configObj.insert("antiDeleteMessages", Config::AntiDeleteMessages());
	configObj.insert("saveEditHistory", Config::SaveEditHistory());
	configObj.insert("socialLinkFixer", Config::SocialLinkFixer());
	configObj.insert("showSecondsInTime", Config::ShowSecondsInTime());
	configObj.insert("customBadgesEnabled", Config::CustomBadgesEnabled());
	configObj.insert("showRegistrationDate", Config::ShowRegistrationDate());
	configObj.insert("showOriginalName", Config::ShowOriginalName());

	QJsonObject root;
	root.insert("app", "WickedGram");
	root.insert("schema_version", 1);
	root.insert("export_timestamp", QDateTime::currentSecsSinceEpoch());
	root.insert("config", configObj);

	const auto dir = QFileInfo(dest).dir();
	if (!dir.exists()) {
		dir.mkpath(u"."_q);
	}

	QFile file(dest);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		return QString();
	}

	file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
	return dest;
}

bool BackupManager::ImportBackup(const QString &filePath) {
	if (filePath.isEmpty() || !QFile::exists(filePath)) {
		return false;
	}

	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly)) {
		return false;
	}

	const auto doc = QJsonDocument::fromJson(file.readAll());
	if (!doc.isObject()) {
		return false;
	}
	const auto root = doc.object();
	const auto appName = root.value("app").toString();
	if (appName != u"WickedGram"_q && appName != u"100gram"_q) {
		// Allow importing standard 100gram settings too!
		return false;
	}

	const auto configObj = root.contains("config")
		? root.value("config").toObject()
		: root;

	if (configObj.contains("bypassContentProtection")) {
		Config::SetBypassContentProtection(configObj.value("bypassContentProtection").toBool());
	}
	if (configObj.contains("ghostMode")) {
		Config::SetGhostMode(configObj.value("ghostMode").toBool());
	}
	if (configObj.contains("stealthTyping")) {
		Config::SetStealthTyping(configObj.value("stealthTyping").toBool());
	}
	if (configObj.contains("antiDeleteMessages")) {
		Config::SetAntiDeleteMessages(configObj.value("antiDeleteMessages").toBool());
	}
	if (configObj.contains("saveEditHistory")) {
		Config::SetSaveEditHistory(configObj.value("saveEditHistory").toBool());
	}
	if (configObj.contains("socialLinkFixer")) {
		Config::SetSocialLinkFixer(configObj.value("socialLinkFixer").toBool());
	}
	if (configObj.contains("showSecondsInTime")) {
		Config::SetShowSecondsInTime(configObj.value("showSecondsInTime").toBool());
	}
	if (configObj.contains("customBadgesEnabled")) {
		Config::SetCustomBadgesEnabled(configObj.value("customBadgesEnabled").toBool());
	}
	if (configObj.contains("showRegistrationDate")) {
		Config::SetShowRegistrationDate(configObj.value("showRegistrationDate").toBool());
	}
	if (configObj.contains("showOriginalName")) {
		Config::SetShowOriginalName(configObj.value("showOriginalName").toBool());
	}

	return true;
}

} // namespace Wicked
