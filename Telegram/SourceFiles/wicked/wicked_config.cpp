/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#include "wicked/wicked_config.h"

#include "storage/localstorage.h"

#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QStringList>
#include <QtCore/QRegularExpression>
#include <algorithm>

namespace Wicked {

Config &Config::Instance() {
	static Config instance;
	return instance;
}

Config::Config() {
	load();
}

QString Config::ConfigPath() {
	return cWorkingDir() + u"tdata/wickedgram_config.json"_q;
}

bool Config::StreamerHideDialogNames() {
	return Instance()._streamerHideDialogNames.current();
}

rpl::producer<bool> Config::StreamerHideDialogNamesValue() {
	return Instance()._streamerHideDialogNames.value();
}

void Config::SetStreamerHideDialogNames(bool value) {
	if (Instance()._streamerHideDialogNames.current() != value) {
		Instance()._streamerHideDialogNames = value;
		Instance().save();
	}
}

bool Config::StreamerHideDialogAvatars() {
	return Instance()._streamerHideDialogAvatars.current();
}

rpl::producer<bool> Config::StreamerHideDialogAvatarsValue() {
	return Instance()._streamerHideDialogAvatars.value();
}

void Config::SetStreamerHideDialogAvatars(bool value) {
	if (Instance()._streamerHideDialogAvatars.current() != value) {
		Instance()._streamerHideDialogAvatars = value;
		Instance().save();
	}
}

bool Config::StreamerHideDialogMessages() {
	return Instance()._streamerHideDialogMessages.current();
}

rpl::producer<bool> Config::StreamerHideDialogMessagesValue() {
	return Instance()._streamerHideDialogMessages.value();
}

void Config::SetStreamerHideDialogMessages(bool value) {
	if (Instance()._streamerHideDialogMessages.current() != value) {
		Instance()._streamerHideDialogMessages = value;
		Instance().save();
	}
}

bool Config::StreamerHideNotificationPreview() {
	return Instance()._streamerHideNotificationPreview.current();
}

rpl::producer<bool> Config::StreamerHideNotificationPreviewValue() {
	return Instance()._streamerHideNotificationPreview.value();
}

void Config::SetStreamerHideNotificationPreview(bool value) {
	if (Instance()._streamerHideNotificationPreview.current() != value) {
		Instance()._streamerHideNotificationPreview = value;
		Instance().save();
	}
}

bool Config::VaultSavePrivate() {
	return Instance()._vaultSavePrivate.current();
}

rpl::producer<bool> Config::VaultSavePrivateValue() {
	return Instance()._vaultSavePrivate.value();
}

void Config::SetVaultSavePrivate(bool value) {
	if (Instance()._vaultSavePrivate.current() != value) {
		Instance()._vaultSavePrivate = value;
		Instance().save();
	}
}

bool Config::VaultSaveGroups() {
	return Instance()._vaultSaveGroups.current();
}

rpl::producer<bool> Config::VaultSaveGroupsValue() {
	return Instance()._vaultSaveGroups.value();
}

void Config::SetVaultSaveGroups(bool value) {
	if (Instance()._vaultSaveGroups.current() != value) {
		Instance()._vaultSaveGroups = value;
		Instance().save();
	}
}

bool Config::VaultSaveChannels() {
	return Instance()._vaultSaveChannels.current();
}

rpl::producer<bool> Config::VaultSaveChannelsValue() {
	return Instance()._vaultSaveChannels.value();
}

void Config::SetVaultSaveChannels(bool value) {
	if (Instance()._vaultSaveChannels.current() != value) {
		Instance()._vaultSaveChannels = value;
		Instance().save();
	}
}

bool Config::VaultSaveBots() {
	return Instance()._vaultSaveBots.current();
}

rpl::producer<bool> Config::VaultSaveBotsValue() {
	return Instance()._vaultSaveBots.value();
}

void Config::SetVaultSaveBots(bool value) {
	if (Instance()._vaultSaveBots.current() != value) {
		Instance()._vaultSaveBots = value;
		Instance().save();
	}
}

bool Config::VaultSaveArchived() {
	return Instance()._vaultSaveArchived.current();
}

rpl::producer<bool> Config::VaultSaveArchivedValue() {
	return Instance()._vaultSaveArchived.value();
}

void Config::SetVaultSaveArchived(bool value) {
	if (Instance()._vaultSaveArchived.current() != value) {
		Instance()._vaultSaveArchived = value;
		Instance().save();
	}
}

bool Config::VaultSavePhotos() {
	return Instance()._vaultSavePhotos.current();
}

rpl::producer<bool> Config::VaultSavePhotosValue() {
	return Instance()._vaultSavePhotos.value();
}

void Config::SetVaultSavePhotos(bool value) {
	if (Instance()._vaultSavePhotos.current() != value) {
		Instance()._vaultSavePhotos = value;
		Instance().save();
	}
}

bool Config::VaultSaveVideos() {
	return Instance()._vaultSaveVideos.current();
}

rpl::producer<bool> Config::VaultSaveVideosValue() {
	return Instance()._vaultSaveVideos.value();
}

void Config::SetVaultSaveVideos(bool value) {
	if (Instance()._vaultSaveVideos.current() != value) {
		Instance()._vaultSaveVideos = value;
		Instance().save();
	}
}

bool Config::VaultSaveVoice() {
	return Instance()._vaultSaveVoice.current();
}

rpl::producer<bool> Config::VaultSaveVoiceValue() {
	return Instance()._vaultSaveVoice.value();
}

void Config::SetVaultSaveVoice(bool value) {
	if (Instance()._vaultSaveVoice.current() != value) {
		Instance()._vaultSaveVoice = value;
		Instance().save();
	}
}

bool Config::VaultSaveFiles() {
	return Instance()._vaultSaveFiles.current();
}

rpl::producer<bool> Config::VaultSaveFilesValue() {
	return Instance()._vaultSaveFiles.value();
}

void Config::SetVaultSaveFiles(bool value) {
	if (Instance()._vaultSaveFiles.current() != value) {
		Instance()._vaultSaveFiles = value;
		Instance().save();
	}
}

int Config::VaultMaxFileMb() {
	return Instance()._vaultMaxFileMb.current();
}

rpl::producer<int> Config::VaultMaxFileMbValue() {
	return Instance()._vaultMaxFileMb.value();
}

void Config::SetVaultMaxFileMb(int value) {
	value = std::clamp(value, 0, 2048);
	if (Instance()._vaultMaxFileMb.current() != value) {
		Instance()._vaultMaxFileMb = value;
		Instance().save();
	}
}

bool Config::VaultChatProtected(const QString &key) {
	return Instance()._vaultProtectedChats.contains(key);
}

const QStringList &Config::VaultProtectedChats() {
	return Instance()._vaultProtectedChats;
}

void Config::SetVaultProtectedChats(const QStringList &keys) {
	static const auto pattern = QRegularExpression(u"^[0-9]+:[0-9]+$"_q);
	if (keys.size() > 10000 || std::any_of(keys.begin(), keys.end(), [&](const QString &key) {
		return !pattern.match(key).hasMatch();
	})) {
		return;
	}
	auto normalized = keys;
	normalized.removeDuplicates();
	if (Instance()._vaultProtectedChats != normalized) {
		Instance()._vaultProtectedChats = std::move(normalized);
		Instance().save();
	}
}

void Config::SetVaultChatProtected(const QString &key, bool value) {
	auto keys = VaultProtectedChats();
	if (value) {
		keys.push_back(key);
	} else {
		keys.removeAll(key);
	}
	SetVaultProtectedChats(keys);
}

bool Config::ConfirmCalls() {
	return Instance()._confirmCalls.current();
}

rpl::producer<bool> Config::ConfirmCallsValue() {
	return Instance()._confirmCalls.value();
}

void Config::SetConfirmCalls(bool enabled) {
	if (Instance()._confirmCalls.current() != enabled) {
		Instance()._confirmCalls = enabled;
		Instance().save();
	}
}

bool Config::ShowWeekdayInDate() {
	return Instance()._showWeekdayInDate.current();
}

rpl::producer<bool> Config::ShowWeekdayInDateValue() {
	return Instance()._showWeekdayInDate.value();
}

void Config::SetShowWeekdayInDate(bool enabled) {
	if (Instance()._showWeekdayInDate.current() != enabled) {
		Instance()._showWeekdayInDate = enabled;
		Instance().save();
	}
}

bool Config::HideShareButton() {
	return Instance()._hideShareButton.current();
}

rpl::producer<bool> Config::HideShareButtonValue() {
	return Instance()._hideShareButton.value();
}

void Config::SetHideShareButton(bool enabled) {
	if (Instance()._hideShareButton.current() != enabled) {
		Instance()._hideShareButton = enabled;
		Instance().save();
	}
}

bool Config::DisableGreetingSticker() {
	return Instance()._disableGreetingSticker.current();
}

rpl::producer<bool> Config::DisableGreetingStickerValue() {
	return Instance()._disableGreetingSticker.value();
}

void Config::SetDisableGreetingSticker(bool enabled) {
	if (Instance()._disableGreetingSticker.current() != enabled) {
		Instance()._disableGreetingSticker = enabled;
		Instance().save();
	}
}

bool Config::BypassContentProtection() {
	return Instance()._bypassContentProtection.current();
}

rpl::producer<bool> Config::BypassContentProtectionValue() {
	return Instance()._bypassContentProtection.value();
}

void Config::SetBypassContentProtection(bool enabled) {
	if (Instance()._bypassContentProtection.current() != enabled) {
		Instance()._bypassContentProtection = enabled;
		Instance().save();
	}
}

bool Config::GhostMode() {
	return Instance()._ghostMode.current();
}

rpl::producer<bool> Config::GhostModeValue() {
	return Instance()._ghostMode.value();
}

void Config::SetGhostMode(bool enabled) {
	if (Instance()._ghostMode.current() != enabled) {
		Instance()._ghostMode = enabled;
		Instance().save();
	}
}

bool Config::StealthTyping() {
	return Instance()._stealthTyping.current();
}

rpl::producer<bool> Config::StealthTypingValue() {
	return Instance()._stealthTyping.value();
}

void Config::SetStealthTyping(bool enabled) {
	if (Instance()._stealthTyping.current() != enabled) {
		Instance()._stealthTyping = enabled;
		Instance().save();
	}
}

bool Config::AntiDeleteMessages() {
	return Instance()._antiDeleteMessages.current();
}

rpl::producer<bool> Config::AntiDeleteMessagesValue() {
	return Instance()._antiDeleteMessages.value();
}

void Config::SetAntiDeleteMessages(bool enabled) {
	if (Instance()._antiDeleteMessages.current() != enabled) {
		Instance()._antiDeleteMessages = enabled;
		Instance().save();
	}
}

bool Config::SaveEditHistory() {
	return Instance()._saveEditHistory.current();
}

rpl::producer<bool> Config::SaveEditHistoryValue() {
	return Instance()._saveEditHistory.value();
}

void Config::SetSaveEditHistory(bool enabled) {
	if (Instance()._saveEditHistory.current() != enabled) {
		Instance()._saveEditHistory = enabled;
		Instance().save();
	}
}

bool Config::SocialLinkFixer() {
	return Instance()._socialLinkFixer.current();
}

rpl::producer<bool> Config::SocialLinkFixerValue() {
	return Instance()._socialLinkFixer.value();
}

void Config::SetSocialLinkFixer(bool enabled) {
	if (Instance()._socialLinkFixer.current() != enabled) {
		Instance()._socialLinkFixer = enabled;
		Instance().save();
	}
}

bool Config::ShowSecondsInTime() {
	return Instance()._showSecondsInTime.current();
}

rpl::producer<bool> Config::ShowSecondsInTimeValue() {
	return Instance()._showSecondsInTime.value();
}

void Config::SetShowSecondsInTime(bool enabled) {
	if (Instance()._showSecondsInTime.current() != enabled) {
		Instance()._showSecondsInTime = enabled;
		Instance().save();
	}
}

bool Config::CustomBadgesEnabled() {
	return Instance()._customBadgesEnabled.current();
}

rpl::producer<bool> Config::CustomBadgesEnabledValue() {
	return Instance()._customBadgesEnabled.value();
}

void Config::SetCustomBadgesEnabled(bool enabled) {
	if (Instance()._customBadgesEnabled.current() != enabled) {
		Instance()._customBadgesEnabled = enabled;
		Instance().save();
	}
}

bool Config::ShowRegistrationDate() {
	return Instance()._showRegistrationDate.current();
}

rpl::producer<bool> Config::ShowRegistrationDateValue() {
	return Instance()._showRegistrationDate.value();
}

void Config::SetShowRegistrationDate(bool enabled) {
	if (Instance()._showRegistrationDate.current() != enabled) {
		Instance()._showRegistrationDate = enabled;
		Instance().save();
	}
}

bool Config::ShowOriginalName() {
	return Instance()._showOriginalName.current();
}

rpl::producer<bool> Config::ShowOriginalNameValue() {
	return Instance()._showOriginalName.value();
}

void Config::SetShowOriginalName(bool enabled) {
	if (Instance()._showOriginalName.current() != enabled) {
		Instance()._showOriginalName = enabled;
		Instance().save();
	}
}

bool Config::UnlimitedAccounts() {
	return Instance()._unlimitedAccounts.current();
}

rpl::producer<bool> Config::UnlimitedAccountsValue() {
	return Instance()._unlimitedAccounts.value();
}

void Config::SetUnlimitedAccounts(bool enabled) {
	if (Instance()._unlimitedAccounts.current() != enabled) {
		Instance()._unlimitedAccounts = enabled;
		Instance().save();
	}
}

bool Config::AutoVaultMedia() {
	return Instance()._autoVaultMedia.current();
}

rpl::producer<bool> Config::AutoVaultMediaValue() {
	return Instance()._autoVaultMedia.value();
}

void Config::SetAutoVaultMedia(bool enabled) {
	if (Instance()._autoVaultMedia.current() != enabled) {
		Instance()._autoVaultMedia = enabled;
		Instance().save();
	}
}

int Config::VaultMaxSizeGb() {
	return Instance()._vaultMaxSizeGb.current();
}

rpl::producer<int> Config::VaultMaxSizeGbValue() {
	return Instance()._vaultMaxSizeGb.value();
}

void Config::SetVaultMaxSizeGb(int gb) {
	gb = std::clamp(gb, 0, 1024);
	if (Instance()._vaultMaxSizeGb.current() != gb) {
		Instance()._vaultMaxSizeGb = gb;
		Instance().save();
	}
}

int Config::VaultTtlDays() {
	return Instance()._vaultTtlDays.current();
}

rpl::producer<int> Config::VaultTtlDaysValue() {
	return Instance()._vaultTtlDays.value();
}

void Config::SetVaultTtlDays(int days) {
	days = std::clamp(days, 0, 3650);
	if (Instance()._vaultTtlDays.current() != days) {
		Instance()._vaultTtlDays = days;
		Instance().save();
	}
}

bool Config::StreamerMode() {
	return Instance()._streamerMode.current();
}

rpl::producer<bool> Config::StreamerModeValue() {
	return Instance()._streamerMode.value();
}

void Config::SetStreamerMode(bool enabled) {
	if (Instance()._streamerMode.current() != enabled) {
		Instance()._streamerMode = enabled;
		Instance().save();
	}
}

bool Config::StreamerHideOtherAccounts() {
	return Instance()._streamerHideOtherAccounts.current();
}

rpl::producer<bool> Config::StreamerHideOtherAccountsValue() {
	return Instance()._streamerHideOtherAccounts.value();
}

void Config::SetStreamerHideOtherAccounts(bool enabled) {
	if (Instance()._streamerHideOtherAccounts.current() != enabled) {
		Instance()._streamerHideOtherAccounts = enabled;
		Instance().save();
	}
}

bool Config::StreamerHidePhoneNumbers() {
	return Instance()._streamerHidePhoneNumbers.current();
}

rpl::producer<bool> Config::StreamerHidePhoneNumbersValue() {
	return Instance()._streamerHidePhoneNumbers.value();
}

void Config::SetStreamerHidePhoneNumbers(bool enabled) {
	if (Instance()._streamerHidePhoneNumbers.current() != enabled) {
		Instance()._streamerHidePhoneNumbers = enabled;
		Instance().save();
	}
}

bool Config::FilterZalgo() {
	return Instance()._filterZalgo.current();
}

rpl::producer<bool> Config::FilterZalgoValue() {
	return Instance()._filterZalgo.value();
}

void Config::SetFilterZalgo(bool enabled) {
	if (Instance()._filterZalgo.current() != enabled) {
		Instance()._filterZalgo = enabled;
		Instance().save();
	}
}

bool Config::CleanUrls() {
	return Instance()._cleanUrls.current();
}

rpl::producer<bool> Config::CleanUrlsValue() {
	return Instance()._cleanUrls.value();
}

void Config::SetCleanUrls(bool enabled) {
	if (Instance()._cleanUrls.current() != enabled) {
		Instance()._cleanUrls = enabled;
		Instance().save();
	}
}

bool Config::LyricsInPlayer() {
	return Instance()._lyricsInPlayer.current();
}

rpl::producer<bool> Config::LyricsInPlayerValue() {
	return Instance()._lyricsInPlayer.value();
}

void Config::SetLyricsInPlayer(bool enabled) {
	if (Instance()._lyricsInPlayer.current() != enabled) {
		Instance()._lyricsInPlayer = enabled;
		Instance().save();
	}
}

NetworkBoostMode Config::DownloadBoost() {
	return Instance()._downloadBoost.current();
}

rpl::producer<NetworkBoostMode> Config::DownloadBoostValue() {
	return Instance()._downloadBoost.value();
}

void Config::SetDownloadBoost(NetworkBoostMode mode) {
	if (Instance()._downloadBoost.current() != mode) {
		Instance()._downloadBoost = mode;
		Instance().save();
	}
}

NetworkBoostMode Config::UploadBoost() {
	return Instance()._uploadBoost.current();
}

rpl::producer<NetworkBoostMode> Config::UploadBoostValue() {
	return Instance()._uploadBoost.value();
}

void Config::SetUploadBoost(NetworkBoostMode mode) {
	if (Instance()._uploadBoost.current() != mode) {
		Instance()._uploadBoost = mode;
		Instance().save();
	}
}

bool Config::UploadBoostFileSizeAware() {
	return Instance()._uploadBoostFileSizeAware.current();
}

rpl::producer<bool> Config::UploadBoostFileSizeAwareValue() {
	return Instance()._uploadBoostFileSizeAware.value();
}

void Config::SetUploadBoostFileSizeAware(bool enabled) {
	if (Instance()._uploadBoostFileSizeAware.current() != enabled) {
		Instance()._uploadBoostFileSizeAware = enabled;
		Instance().save();
	}
}

QString Config::FixSocialLinks(QString text) {
	if (!SocialLinkFixer() || text.isEmpty()) {
		return text;
	}
	auto result = text;
	if (result.contains(u"twitter.com/"_q)) {
		result.replace(u"https://twitter.com/"_q, u"https://fxtwitter.com/"_q);
		result.replace(u"http://twitter.com/"_q, u"https://fxtwitter.com/"_q);
	}
	if (result.contains(u"x.com/"_q)) {
		result.replace(u"https://x.com/"_q, u"https://fixupx.com/"_q);
		result.replace(u"http://x.com/"_q, u"https://fixupx.com/"_q);
	}
	if (result.contains(u"instagram.com/"_q)) {
		result.replace(u"https://www.instagram.com/"_q, u"https://ddinstagram.com/"_q);
		result.replace(u"https://instagram.com/"_q, u"https://ddinstagram.com/"_q);
		result.replace(u"http://instagram.com/"_q, u"https://ddinstagram.com/"_q);
	}
	if (result.contains(u"tiktok.com/"_q)) {
		result.replace(u"https://www.tiktok.com/"_q, u"https://vxtiktok.com/"_q);
		result.replace(u"https://tiktok.com/"_q, u"https://vxtiktok.com/"_q);
	}
	if (result.contains(u"reddit.com/"_q)) {
		result.replace(u"https://www.reddit.com/"_q, u"https://rxddit.com/"_q);
		result.replace(u"https://reddit.com/"_q, u"https://rxddit.com/"_q);
	}
	if (result.contains(u"pixiv.net/"_q)) {
		result.replace(u"https://www.pixiv.net/"_q, u"https://phixiv.net/"_q);
		result.replace(u"https://pixiv.net/"_q, u"https://phixiv.net/"_q);
	}
	return result;
}

QString Config::CleanUrl(QString url) {
	if (url.isEmpty() || (!url.startsWith(u"http://"_q) && !url.startsWith(u"https://"_q))) {
		return url;
	}
	const auto qIndex = url.indexOf('?');
	if (qIndex < 0) {
		return url;
	}
	const auto base = url.left(qIndex);
	auto query = url.mid(qIndex + 1);
	auto fragment = QString();
	const auto fIndex = query.indexOf('#');
	if (fIndex >= 0) {
		fragment = query.mid(fIndex);
		query = query.left(fIndex);
	}
	const auto pairs = query.split('&', Qt::SkipEmptyParts);
	QStringList newPairs;
	static const auto tracking = std::vector<QString>{
		u"utm_source"_q, u"utm_medium"_q, u"utm_campaign"_q,
		u"utm_term"_q, u"utm_content"_q, u"utm_id"_q,
		u"fbclid"_q, u"gclid"_q, u"gbraid"_q, u"wbraid"_q,
		u"si"_q, u"igshid"_q, u"ref"_q, u"ref_src"_q,
		u"yclid"_q, u"ysclid"_q, u"mc_eid"_q, u"_hsenc"_q, u"_hsmi"_q
	};
	for (const auto &pair : pairs) {
		const auto eqIndex = pair.indexOf('=');
		const auto key = (eqIndex >= 0 ? pair.left(eqIndex) : pair).toLower();
		bool isTracking = false;
		if (key.startsWith(u"utm_"_q)) {
			isTracking = true;
		} else {
			for (const auto &t : tracking) {
				if (key == t) {
					isTracking = true;
					break;
				}
			}
		}
		if (!isTracking) {
			newPairs.append(pair);
		}
	}
	if (newPairs.isEmpty()) {
		return base + fragment;
	}
	return base + '?' + newPairs.join('&') + fragment;
}

QString Config::CleanUrlsInText(QString text) {
	if (!CleanUrls() || text.isEmpty()) {
		return text;
	}
	const auto tokens = text.split(' ');
	QStringList cleaned;
	for (const auto &tok : tokens) {
		if (tok.startsWith(u"http://"_q) || tok.startsWith(u"https://"_q)) {
			cleaned.append(CleanUrl(tok));
		} else {
			cleaned.append(tok);
		}
	}
	return cleaned.join(' ');
}

QString Config::FilterZalgoText(QString text) {
	if (text.isEmpty() || !FilterZalgo()) {
		return text;
	}
	QString result;
	result.reserve(text.size());
	int consecutiveMarks = 0;
	for (int i = 0; i < text.size(); ++i) {
		const auto ch = text.at(i);
		const auto ucs4 = ch.unicode();
		if (ucs4 == 1564 || ucs4 == 8206 || ucs4 == 8207
			|| (ucs4 >= 8234 && ucs4 <= 8238)
			|| (ucs4 >= 8294 && ucs4 <= 8297)) {
			continue;
		}
		const bool isMark = (ucs4 >= 768 && ucs4 <= 879)
			|| (ucs4 >= 6832 && ucs4 <= 6911)
			|| (ucs4 >= 7616 && ucs4 <= 7679)
			|| (ucs4 >= 8400 && ucs4 <= 8447)
			|| (ucs4 >= 65056 && ucs4 <= 65071);
		if (isMark) {
			consecutiveMarks++;
			if (consecutiveMarks > 2) {
				continue;
			}
		} else {
			consecutiveMarks = 0;
		}
		result.append(ch);
	}
	return result;
}

bool Config::IsCustomBadgeUser(uint64 userId) {
	if (!CustomBadgesEnabled()) {
		return false;
	}
	// 1211607237 is Lead Developer from 100gram/badges.json
	return (userId == 1211607237ULL);
}

void Config::load() {
	if (_loaded) {
		return;
	}
	_loaded = true;

	QFile file(ConfigPath());
	if (!file.open(QIODevice::ReadOnly)) {
		return;
	}

	const auto doc = QJsonDocument::fromJson(file.readAll());
	if (!doc.isObject()) {
		return;
	}
	const auto obj = doc.object();

	if (obj.contains("bypassContentProtection")) {
		_bypassContentProtection = obj.value("bypassContentProtection").toBool(false);
	}
	if (obj.contains("ghostMode")) {
		_ghostMode = obj.value("ghostMode").toBool(false);
	}
	if (obj.contains("stealthTyping")) {
		_stealthTyping = obj.value("stealthTyping").toBool(false);
	}
	if (obj.contains("antiDeleteMessages")) {
		_antiDeleteMessages = obj.value("antiDeleteMessages").toBool(true);
	}
	if (obj.contains("saveEditHistory")) {
		_saveEditHistory = obj.value("saveEditHistory").toBool(true);
	}
	if (obj.contains("socialLinkFixer")) {
		_socialLinkFixer = obj.value("socialLinkFixer").toBool(true);
	}
	if (obj.contains("showSecondsInTime")) {
		_showSecondsInTime = obj.value("showSecondsInTime").toBool(false);
	}
	if (obj.contains("customBadgesEnabled")) {
		_customBadgesEnabled = obj.value("customBadgesEnabled").toBool(true);
	}
	if (obj.contains("showRegistrationDate")) {
		_showRegistrationDate = obj.value("showRegistrationDate").toBool(true);
	}
	if (obj.contains("showOriginalName")) {
		_showOriginalName = obj.value("showOriginalName").toBool(true);
	}
	if (obj.contains("unlimitedAccounts")) {
		_unlimitedAccounts = obj.value("unlimitedAccounts").toBool(true);
	}
	if (obj.contains("autoVaultMedia")) {
		_autoVaultMedia = obj.value("autoVaultMedia").toBool(false);
	}
	if (obj.contains("vaultMaxSizeGb")) {
		_vaultMaxSizeGb = std::clamp(obj.value("vaultMaxSizeGb").toInt(0), 0, 1024);
	}
	if (obj.contains("vaultTtlDays")) {
		_vaultTtlDays = std::clamp(obj.value("vaultTtlDays").toInt(0), 0, 3650);
	}
	if (obj.contains("streamerMode")) {
		_streamerMode = obj.value("streamerMode").toBool(false);
	}
	if (obj.contains("streamerHideOtherAccounts")) {
		_streamerHideOtherAccounts = obj.value("streamerHideOtherAccounts").toBool(true);
	}
	if (obj.contains("streamerHidePhoneNumbers")) {
		_streamerHidePhoneNumbers = obj.value("streamerHidePhoneNumbers").toBool(true);
	}
	if (obj.contains("filterZalgo")) {
		_filterZalgo = obj.value("filterZalgo").toBool(true);
	}
	if (obj.contains("cleanUrls")) {
		_cleanUrls = obj.value("cleanUrls").toBool(true);
	}
	if (obj.contains("lyricsInPlayer")) {
		_lyricsInPlayer = obj.value("lyricsInPlayer").toBool(true);
	}
	if (obj.contains("downloadBoost")) {
		const auto val = obj.value("downloadBoost").toInt(int(NetworkBoostMode::Fast));
		_downloadBoost = static_cast<NetworkBoostMode>(std::clamp(val, 0, 2));
	}
	if (obj.contains("uploadBoost")) {
		const auto val = obj.value("uploadBoost").toInt(int(NetworkBoostMode::Fast));
		_uploadBoost = static_cast<NetworkBoostMode>(std::clamp(val, 0, 2));
	}
	if (obj.contains("uploadBoostFileSizeAware")) {
		_uploadBoostFileSizeAware = obj.value("uploadBoostFileSizeAware").toBool(true);
	}
	if (obj.value("confirmCalls").isBool()) {
		_confirmCalls = obj.value("confirmCalls").toBool();
	}
	if (obj.value("showWeekdayInDate").isBool()) {
		_showWeekdayInDate = obj.value("showWeekdayInDate").toBool();
	}
	if (obj.value("hideShareButton").isBool()) {
		_hideShareButton = obj.value("hideShareButton").toBool();
	}
	if (obj.value("disableGreetingSticker").isBool()) {
		_disableGreetingSticker = obj.value("disableGreetingSticker").toBool();
	}
	if (obj.value("streamerHideDialogNames").isBool()) {
		_streamerHideDialogNames = obj.value("streamerHideDialogNames").toBool();
	}
	if (obj.value("streamerHideDialogAvatars").isBool()) {
		_streamerHideDialogAvatars = obj.value("streamerHideDialogAvatars").toBool();
	}
	if (obj.value("streamerHideDialogMessages").isBool()) {
		_streamerHideDialogMessages = obj.value("streamerHideDialogMessages").toBool();
	}
	if (obj.value("streamerHideNotificationPreview").isBool()) {
		_streamerHideNotificationPreview = obj.value("streamerHideNotificationPreview").toBool();
	}
	if (obj.value("vaultSavePrivate").isBool()) {
		_vaultSavePrivate = obj.value("vaultSavePrivate").toBool();
	}
	if (obj.value("vaultSaveGroups").isBool()) {
		_vaultSaveGroups = obj.value("vaultSaveGroups").toBool();
	}
	if (obj.value("vaultSaveChannels").isBool()) {
		_vaultSaveChannels = obj.value("vaultSaveChannels").toBool();
	}
	if (obj.value("vaultSaveBots").isBool()) {
		_vaultSaveBots = obj.value("vaultSaveBots").toBool();
	}
	if (obj.value("vaultSaveArchived").isBool()) {
		_vaultSaveArchived = obj.value("vaultSaveArchived").toBool();
	}
	if (obj.value("vaultSavePhotos").isBool()) {
		_vaultSavePhotos = obj.value("vaultSavePhotos").toBool();
	}
	if (obj.value("vaultSaveVideos").isBool()) {
		_vaultSaveVideos = obj.value("vaultSaveVideos").toBool();
	}
	if (obj.value("vaultSaveVoice").isBool()) {
		_vaultSaveVoice = obj.value("vaultSaveVoice").toBool();
	}
	if (obj.value("vaultSaveFiles").isBool()) {
		_vaultSaveFiles = obj.value("vaultSaveFiles").toBool();
	}
	_vaultMaxFileMb = std::clamp(obj.value("vaultMaxFileMb").toInt(0), 0, 2048);
	for (const auto &entry : obj.value("vaultProtectedChats").toArray()) {
		const auto key = entry.toString();
		if (QRegularExpression(u"^[0-9]+:[0-9]+$"_q).match(key).hasMatch()) {
			_vaultProtectedChats.push_back(key);
		}
	}
	_vaultProtectedChats.removeDuplicates();
}

void Config::save() {
	const auto path = ConfigPath();
	const auto dir = QFileInfo(path).dir();
	if (!dir.exists()) {
		dir.mkpath(u"."_q);
	}

	QJsonObject obj;
	obj.insert("bypassContentProtection", _bypassContentProtection.current());
	obj.insert("ghostMode", _ghostMode.current());
	obj.insert("stealthTyping", _stealthTyping.current());
	obj.insert("antiDeleteMessages", _antiDeleteMessages.current());
	obj.insert("saveEditHistory", _saveEditHistory.current());
	obj.insert("socialLinkFixer", _socialLinkFixer.current());
	obj.insert("showSecondsInTime", _showSecondsInTime.current());
	obj.insert("customBadgesEnabled", _customBadgesEnabled.current());
	obj.insert("showRegistrationDate", _showRegistrationDate.current());
	obj.insert("showOriginalName", _showOriginalName.current());
	obj.insert("unlimitedAccounts", _unlimitedAccounts.current());
	obj.insert("autoVaultMedia", _autoVaultMedia.current());
	obj.insert("vaultMaxSizeGb", _vaultMaxSizeGb.current());
	obj.insert("vaultTtlDays", _vaultTtlDays.current());
	obj.insert("streamerMode", _streamerMode.current());
	obj.insert("streamerHideOtherAccounts", _streamerHideOtherAccounts.current());
	obj.insert("streamerHidePhoneNumbers", _streamerHidePhoneNumbers.current());
	obj.insert("filterZalgo", _filterZalgo.current());
	obj.insert("cleanUrls", _cleanUrls.current());
	obj.insert("lyricsInPlayer", _lyricsInPlayer.current());
	obj.insert("downloadBoost", int(_downloadBoost.current()));
	obj.insert("uploadBoost", int(_uploadBoost.current()));
	obj.insert("uploadBoostFileSizeAware", _uploadBoostFileSizeAware.current());

	obj.insert("confirmCalls", _confirmCalls.current());
	obj.insert("showWeekdayInDate", _showWeekdayInDate.current());
	obj.insert("hideShareButton", _hideShareButton.current());
	obj.insert("disableGreetingSticker", _disableGreetingSticker.current());

	obj.insert("streamerHideDialogNames", _streamerHideDialogNames.current());
	obj.insert("streamerHideDialogAvatars", _streamerHideDialogAvatars.current());
	obj.insert("streamerHideDialogMessages", _streamerHideDialogMessages.current());
	obj.insert("streamerHideNotificationPreview", _streamerHideNotificationPreview.current());
	obj.insert("vaultSavePrivate", _vaultSavePrivate.current());
	obj.insert("vaultSaveGroups", _vaultSaveGroups.current());
	obj.insert("vaultSaveChannels", _vaultSaveChannels.current());
	obj.insert("vaultSaveBots", _vaultSaveBots.current());
	obj.insert("vaultSaveArchived", _vaultSaveArchived.current());
	obj.insert("vaultSavePhotos", _vaultSavePhotos.current());
	obj.insert("vaultSaveVideos", _vaultSaveVideos.current());
	obj.insert("vaultSaveVoice", _vaultSaveVoice.current());
	obj.insert("vaultSaveFiles", _vaultSaveFiles.current());
	obj.insert("vaultMaxFileMb", _vaultMaxFileMb.current());
	obj.insert("vaultProtectedChats", QJsonArray::fromStringList(_vaultProtectedChats));

	QFile file(path);
	if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
	}
}

} // namespace Wicked
