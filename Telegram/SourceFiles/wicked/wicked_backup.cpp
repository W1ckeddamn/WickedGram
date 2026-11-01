/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#include "wicked/wicked_backup.h"

#include "wicked/wicked_config.h"
#include "storage/localstorage.h"

#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QDateTime>

namespace Wicked {

QString BackupManager::DefaultBackupDirectory() {
	const auto dir = cWorkingDir() + u"tdata/backups/"_q;
	QDir().mkpath(dir);
	return dir;
}

QString BackupManager::ExportBackup(const QString &destinationPath) {
	const auto dest = destinationPath.isEmpty()
		? (DefaultBackupDirectory() + u"wickedgram_settings_"_q
			+ QString::number(QDateTime::currentSecsSinceEpoch())
			+ u".json"_q)
		: destinationPath;

	Config::Instance().load();

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
	configObj.insert("unlimitedAccounts", Config::UnlimitedAccounts());
	configObj.insert("autoVaultMedia", Config::AutoVaultMedia());
	configObj.insert("vaultMaxSizeGb", Config::VaultMaxSizeGb());
	configObj.insert("vaultTtlDays", Config::VaultTtlDays());
	configObj.insert("streamerMode", Config::StreamerMode());
	configObj.insert("streamerHideOtherAccounts", Config::StreamerHideOtherAccounts());
	configObj.insert("streamerHidePhoneNumbers", Config::StreamerHidePhoneNumbers());
	configObj.insert("filterZalgo", Config::FilterZalgo());
	configObj.insert("cleanUrls", Config::CleanUrls());

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
	if (configObj.contains("unlimitedAccounts")) {
		Config::SetUnlimitedAccounts(configObj.value("unlimitedAccounts").toBool());
	}
	if (configObj.contains("autoVaultMedia")) {
		Config::SetAutoVaultMedia(configObj.value("autoVaultMedia").toBool());
	}
	if (configObj.contains("vaultMaxSizeGb")) {
		Config::SetVaultMaxSizeGb(configObj.value("vaultMaxSizeGb").toInt());
	}
	if (configObj.contains("vaultTtlDays")) {
		Config::SetVaultTtlDays(configObj.value("vaultTtlDays").toInt());
	}
	if (configObj.contains("streamerMode")) {
		Config::SetStreamerMode(configObj.value("streamerMode").toBool());
	}
	if (configObj.contains("streamerHideOtherAccounts")) {
		Config::SetStreamerHideOtherAccounts(configObj.value("streamerHideOtherAccounts").toBool());
	}
	if (configObj.contains("streamerHidePhoneNumbers")) {
		Config::SetStreamerHidePhoneNumbers(configObj.value("streamerHidePhoneNumbers").toBool());
	}
	if (configObj.contains("filterZalgo")) {
		Config::SetFilterZalgo(configObj.value("filterZalgo").toBool());
	}
	if (configObj.contains("cleanUrls")) {
		Config::SetCleanUrls(configObj.value("cleanUrls").toBool());
	}

	return true;
}

} // namespace Wicked
