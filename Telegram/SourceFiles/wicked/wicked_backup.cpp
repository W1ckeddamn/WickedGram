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
#include <QtCore/QJsonArray>
#include <QtCore/QRegularExpression>
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
	configObj.insert("lyricsInPlayer", Config::LyricsInPlayer());
	configObj.insert("downloadBoost", int(Config::DownloadBoost()));
	configObj.insert("uploadBoost", int(Config::UploadBoost()));
	configObj.insert("uploadBoostFileSizeAware", Config::UploadBoostFileSizeAware());

	configObj.insert("confirmCalls", Config::ConfirmCalls());
	configObj.insert("showWeekdayInDate", Config::ShowWeekdayInDate());
	configObj.insert("hideShareButton", Config::HideShareButton());
	configObj.insert("disableGreetingSticker", Config::DisableGreetingSticker());

	configObj.insert("streamerHideDialogNames", Config::StreamerHideDialogNames());
	configObj.insert("streamerHideDialogAvatars", Config::StreamerHideDialogAvatars());
	configObj.insert("streamerHideDialogMessages", Config::StreamerHideDialogMessages());
	configObj.insert("streamerHideNotificationPreview", Config::StreamerHideNotificationPreview());
	configObj.insert("vaultSavePrivate", Config::VaultSavePrivate());
	configObj.insert("vaultSaveGroups", Config::VaultSaveGroups());
	configObj.insert("vaultSaveChannels", Config::VaultSaveChannels());
	configObj.insert("vaultSaveBots", Config::VaultSaveBots());
	configObj.insert("vaultSaveArchived", Config::VaultSaveArchived());
	configObj.insert("vaultSavePhotos", Config::VaultSavePhotos());
	configObj.insert("vaultSaveVideos", Config::VaultSaveVideos());
	configObj.insert("vaultSaveVoice", Config::VaultSaveVoice());
	configObj.insert("vaultSaveFiles", Config::VaultSaveFiles());
	configObj.insert("vaultMaxFileMb", Config::VaultMaxFileMb());
	configObj.insert("vaultProtectedChats", QJsonArray::fromStringList(Config::VaultProtectedChats()));

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
	if (configObj.contains("lyricsInPlayer")) {
		Config::SetLyricsInPlayer(configObj.value("lyricsInPlayer").toBool());
	}
	if (configObj.contains("downloadBoost")) {
		const auto val = configObj.value("downloadBoost").toInt(int(NetworkBoostMode::Fast));
		const auto mode = (val == 3) ? NetworkBoostMode::Fast : static_cast<NetworkBoostMode>(std::clamp(val, 0, 2));
		Config::SetDownloadBoost(mode);
	}
	if (configObj.contains("uploadBoost")) {
		const auto val = configObj.value("uploadBoost").toInt(int(NetworkBoostMode::Fast));
		const auto mode = (val == 3) ? NetworkBoostMode::Fast : static_cast<NetworkBoostMode>(std::clamp(val, 0, 2));
		Config::SetUploadBoost(mode);
	}
	if (configObj.contains("uploadBoostFileSizeAware")) {
		Config::SetUploadBoostFileSizeAware(configObj.value("uploadBoostFileSizeAware").toBool(true));
	}

	if (configObj.value("confirmCalls").isBool()) {
		Config::SetConfirmCalls(configObj.value("confirmCalls").toBool());
	}
	if (configObj.value("showWeekdayInDate").isBool()) {
		Config::SetShowWeekdayInDate(configObj.value("showWeekdayInDate").toBool());
	}
	if (configObj.value("hideShareButton").isBool()) {
		Config::SetHideShareButton(configObj.value("hideShareButton").toBool());
	}
	if (configObj.value("disableGreetingSticker").isBool()) {
		Config::SetDisableGreetingSticker(configObj.value("disableGreetingSticker").toBool());
	}

	if (configObj.value("streamerHideDialogNames").isBool()) {
		Config::SetStreamerHideDialogNames(configObj.value("streamerHideDialogNames").toBool());
	}
	if (configObj.value("streamerHideDialogAvatars").isBool()) {
		Config::SetStreamerHideDialogAvatars(configObj.value("streamerHideDialogAvatars").toBool());
	}
	if (configObj.value("streamerHideDialogMessages").isBool()) {
		Config::SetStreamerHideDialogMessages(configObj.value("streamerHideDialogMessages").toBool());
	}
	if (configObj.value("streamerHideNotificationPreview").isBool()) {
		Config::SetStreamerHideNotificationPreview(configObj.value("streamerHideNotificationPreview").toBool());
	}
	if (configObj.value("vaultSavePrivate").isBool()) {
		Config::SetVaultSavePrivate(configObj.value("vaultSavePrivate").toBool());
	}
	if (configObj.value("vaultSaveGroups").isBool()) {
		Config::SetVaultSaveGroups(configObj.value("vaultSaveGroups").toBool());
	}
	if (configObj.value("vaultSaveChannels").isBool()) {
		Config::SetVaultSaveChannels(configObj.value("vaultSaveChannels").toBool());
	}
	if (configObj.value("vaultSaveBots").isBool()) {
		Config::SetVaultSaveBots(configObj.value("vaultSaveBots").toBool());
	}
	if (configObj.value("vaultSaveArchived").isBool()) {
		Config::SetVaultSaveArchived(configObj.value("vaultSaveArchived").toBool());
	}
	if (configObj.value("vaultSavePhotos").isBool()) {
		Config::SetVaultSavePhotos(configObj.value("vaultSavePhotos").toBool());
	}
	if (configObj.value("vaultSaveVideos").isBool()) {
		Config::SetVaultSaveVideos(configObj.value("vaultSaveVideos").toBool());
	}
	if (configObj.value("vaultSaveVoice").isBool()) {
		Config::SetVaultSaveVoice(configObj.value("vaultSaveVoice").toBool());
	}
	if (configObj.value("vaultSaveFiles").isBool()) {
		Config::SetVaultSaveFiles(configObj.value("vaultSaveFiles").toBool());
	}
	if (configObj.value("vaultMaxFileMb").isDouble()) {
		Config::SetVaultMaxFileMb(configObj.value("vaultMaxFileMb").toInt(0));
	}
	if (configObj.value("vaultProtectedChats").isArray()) {
		auto keys = QStringList();
		const auto pattern = QRegularExpression(u"^[0-9]+:[0-9]+$"_q);
		auto valid = true;
		for (const auto &entry : configObj.value("vaultProtectedChats").toArray()) {
			const auto key = entry.toString();
			if (!pattern.match(key).hasMatch()) {
				valid = false;
				break;
			}
			keys.push_back(key);
		}
		if (valid) {
			Config::SetVaultProtectedChats(keys);
		}
	}

	return true;
}

} // namespace Wicked
