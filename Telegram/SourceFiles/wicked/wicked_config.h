/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#pragma once

#include <rpl/variable.h>
#include <rpl/producer.h>
#include <QtCore/QString>
#include <QtCore/QStringList>

namespace Wicked {

enum class NetworkBoostMode {
	Off = 0,
	Fast = 1,
	Ultra = 2,
};

class Config final {
public:
	static Config &Instance();

	[[nodiscard]] static bool BypassContentProtection();
	[[nodiscard]] static rpl::producer<bool> BypassContentProtectionValue();
	static void SetBypassContentProtection(bool enabled);

	[[nodiscard]] static bool GhostMode();
	[[nodiscard]] static rpl::producer<bool> GhostModeValue();
	static void SetGhostMode(bool enabled);

	[[nodiscard]] static bool StealthTyping();
	[[nodiscard]] static rpl::producer<bool> StealthTypingValue();
	static void SetStealthTyping(bool enabled);

	[[nodiscard]] static bool AntiDeleteMessages();
	[[nodiscard]] static rpl::producer<bool> AntiDeleteMessagesValue();
	static void SetAntiDeleteMessages(bool enabled);

	[[nodiscard]] static bool SaveEditHistory();
	[[nodiscard]] static rpl::producer<bool> SaveEditHistoryValue();
	static void SetSaveEditHistory(bool enabled);

	[[nodiscard]] static bool SocialLinkFixer();
	[[nodiscard]] static rpl::producer<bool> SocialLinkFixerValue();
	static void SetSocialLinkFixer(bool enabled);

	[[nodiscard]] static bool ShowSecondsInTime();
	[[nodiscard]] static rpl::producer<bool> ShowSecondsInTimeValue();
	static void SetShowSecondsInTime(bool enabled);

	[[nodiscard]] static bool CustomBadgesEnabled();
	[[nodiscard]] static rpl::producer<bool> CustomBadgesEnabledValue();
	static void SetCustomBadgesEnabled(bool enabled);

	[[nodiscard]] static bool ShowRegistrationDate();
	[[nodiscard]] static rpl::producer<bool> ShowRegistrationDateValue();
	static void SetShowRegistrationDate(bool enabled);

	[[nodiscard]] static bool ShowOriginalName();
	[[nodiscard]] static rpl::producer<bool> ShowOriginalNameValue();
	static void SetShowOriginalName(bool enabled);

	[[nodiscard]] static bool UnlimitedAccounts();
	[[nodiscard]] static rpl::producer<bool> UnlimitedAccountsValue();
	static void SetUnlimitedAccounts(bool enabled);

	[[nodiscard]] static bool AutoVaultMedia();
	[[nodiscard]] static rpl::producer<bool> AutoVaultMediaValue();
	static void SetAutoVaultMedia(bool enabled);

	[[nodiscard]] static int VaultMaxSizeGb();
	[[nodiscard]] static rpl::producer<int> VaultMaxSizeGbValue();
	static void SetVaultMaxSizeGb(int gb);

	[[nodiscard]] static int VaultTtlDays();
	[[nodiscard]] static rpl::producer<int> VaultTtlDaysValue();
	static void SetVaultTtlDays(int days);

	[[nodiscard]] static bool StreamerMode();
	[[nodiscard]] static rpl::producer<bool> StreamerModeValue();
	static void SetStreamerMode(bool enabled);

	[[nodiscard]] static bool StreamerHideOtherAccounts();
	[[nodiscard]] static rpl::producer<bool> StreamerHideOtherAccountsValue();
	static void SetStreamerHideOtherAccounts(bool enabled);

	[[nodiscard]] static bool StreamerHidePhoneNumbers();
	[[nodiscard]] static rpl::producer<bool> StreamerHidePhoneNumbersValue();
	static void SetStreamerHidePhoneNumbers(bool enabled);

	[[nodiscard]] static bool FilterZalgo();
	[[nodiscard]] static rpl::producer<bool> FilterZalgoValue();
	static void SetFilterZalgo(bool enabled);

	[[nodiscard]] static bool CleanUrls();
	[[nodiscard]] static rpl::producer<bool> CleanUrlsValue();
	static void SetCleanUrls(bool enabled);

	[[nodiscard]] static bool LyricsInPlayer();
	[[nodiscard]] static rpl::producer<bool> LyricsInPlayerValue();
	static void SetLyricsInPlayer(bool enabled);

	[[nodiscard]] static NetworkBoostMode DownloadBoost();
	[[nodiscard]] static rpl::producer<NetworkBoostMode> DownloadBoostValue();
	static void SetDownloadBoost(NetworkBoostMode mode);

	[[nodiscard]] static NetworkBoostMode UploadBoost();
	[[nodiscard]] static rpl::producer<NetworkBoostMode> UploadBoostValue();
	static void SetUploadBoost(NetworkBoostMode mode);

	[[nodiscard]] static bool UploadBoostFileSizeAware();
	[[nodiscard]] static rpl::producer<bool> UploadBoostFileSizeAwareValue();
	static void SetUploadBoostFileSizeAware(bool enabled);

	[[nodiscard]] static bool ConfirmCalls();
	[[nodiscard]] static rpl::producer<bool> ConfirmCallsValue();
	static void SetConfirmCalls(bool enabled);

	[[nodiscard]] static bool ShowWeekdayInDate();
	[[nodiscard]] static rpl::producer<bool> ShowWeekdayInDateValue();
	static void SetShowWeekdayInDate(bool enabled);

	[[nodiscard]] static bool HideShareButton();
	[[nodiscard]] static rpl::producer<bool> HideShareButtonValue();
	static void SetHideShareButton(bool enabled);

	[[nodiscard]] static bool DisableGreetingSticker();
	[[nodiscard]] static rpl::producer<bool> DisableGreetingStickerValue();
	static void SetDisableGreetingSticker(bool enabled);

	[[nodiscard]] static bool StreamerHideDialogNames();
	[[nodiscard]] static rpl::producer<bool> StreamerHideDialogNamesValue();
	static void SetStreamerHideDialogNames(bool value);

	[[nodiscard]] static bool StreamerHideDialogAvatars();
	[[nodiscard]] static rpl::producer<bool> StreamerHideDialogAvatarsValue();
	static void SetStreamerHideDialogAvatars(bool value);

	[[nodiscard]] static bool StreamerHideDialogMessages();
	[[nodiscard]] static rpl::producer<bool> StreamerHideDialogMessagesValue();
	static void SetStreamerHideDialogMessages(bool value);

	[[nodiscard]] static bool StreamerHideNotificationPreview();
	[[nodiscard]] static rpl::producer<bool> StreamerHideNotificationPreviewValue();
	static void SetStreamerHideNotificationPreview(bool value);

	[[nodiscard]] static bool VaultSavePrivate();
	[[nodiscard]] static rpl::producer<bool> VaultSavePrivateValue();
	static void SetVaultSavePrivate(bool value);

	[[nodiscard]] static bool VaultSaveGroups();
	[[nodiscard]] static rpl::producer<bool> VaultSaveGroupsValue();
	static void SetVaultSaveGroups(bool value);

	[[nodiscard]] static bool VaultSaveChannels();
	[[nodiscard]] static rpl::producer<bool> VaultSaveChannelsValue();
	static void SetVaultSaveChannels(bool value);

	[[nodiscard]] static bool VaultSaveBots();
	[[nodiscard]] static rpl::producer<bool> VaultSaveBotsValue();
	static void SetVaultSaveBots(bool value);

	[[nodiscard]] static bool VaultSaveArchived();
	[[nodiscard]] static rpl::producer<bool> VaultSaveArchivedValue();
	static void SetVaultSaveArchived(bool value);

	[[nodiscard]] static bool VaultSavePhotos();
	[[nodiscard]] static rpl::producer<bool> VaultSavePhotosValue();
	static void SetVaultSavePhotos(bool value);

	[[nodiscard]] static bool VaultSaveVideos();
	[[nodiscard]] static rpl::producer<bool> VaultSaveVideosValue();
	static void SetVaultSaveVideos(bool value);

	[[nodiscard]] static bool VaultSaveVoice();
	[[nodiscard]] static rpl::producer<bool> VaultSaveVoiceValue();
	static void SetVaultSaveVoice(bool value);

	[[nodiscard]] static bool VaultSaveFiles();
	[[nodiscard]] static rpl::producer<bool> VaultSaveFilesValue();
	static void SetVaultSaveFiles(bool value);

	[[nodiscard]] static int VaultMaxFileMb();
	[[nodiscard]] static rpl::producer<int> VaultMaxFileMbValue();
	static void SetVaultMaxFileMb(int value);

	[[nodiscard]] static bool VaultChatProtected(const QString &key);
	static void SetVaultChatProtected(const QString &key, bool value);
	static void SetVaultProtectedChats(const QStringList &keys);
	[[nodiscard]] static const QStringList &VaultProtectedChats();

	[[nodiscard]] static QString FixSocialLinks(QString text);
	[[nodiscard]] static QString CleanUrl(QString url);
	[[nodiscard]] static QString CleanUrlsInText(QString text);
	[[nodiscard]] static QString FilterZalgoText(QString text);
	[[nodiscard]] static bool IsCustomBadgeUser(uint64 userId);

	void load();
	void save();

private:
	Config();
	~Config() = default;

	Config(const Config &) = delete;
	Config &operator=(const Config &) = delete;

	static QString ConfigPath();

	rpl::variable<bool> _bypassContentProtection = false;
	rpl::variable<bool> _ghostMode = false;
	rpl::variable<bool> _stealthTyping = false;
	rpl::variable<bool> _antiDeleteMessages = true;
	rpl::variable<bool> _saveEditHistory = true;
	rpl::variable<bool> _socialLinkFixer = true;
	rpl::variable<bool> _showSecondsInTime = false;
	rpl::variable<bool> _customBadgesEnabled = true;
	rpl::variable<bool> _showRegistrationDate = true;
	rpl::variable<bool> _showOriginalName = true;
	rpl::variable<bool> _unlimitedAccounts = true;
	rpl::variable<bool> _autoVaultMedia = false;
	rpl::variable<int> _vaultMaxSizeGb = 0;
	rpl::variable<int> _vaultTtlDays = 0;
	rpl::variable<bool> _streamerMode = false;
	rpl::variable<bool> _streamerHideOtherAccounts = true;
	rpl::variable<bool> _streamerHidePhoneNumbers = true;
	rpl::variable<bool> _filterZalgo = true;
	rpl::variable<bool> _cleanUrls = true;
	rpl::variable<bool> _lyricsInPlayer = true;
	rpl::variable<NetworkBoostMode> _downloadBoost = NetworkBoostMode::Fast;
	rpl::variable<NetworkBoostMode> _uploadBoost = NetworkBoostMode::Fast;
	rpl::variable<bool> _uploadBoostFileSizeAware = true;

	rpl::variable<bool> _confirmCalls = false;
	rpl::variable<bool> _showWeekdayInDate = false;
	rpl::variable<bool> _hideShareButton = false;
	rpl::variable<bool> _disableGreetingSticker = false;

	rpl::variable<bool> _streamerHideDialogNames = true;
	rpl::variable<bool> _streamerHideDialogAvatars = true;
	rpl::variable<bool> _streamerHideDialogMessages = true;
	rpl::variable<bool> _streamerHideNotificationPreview = true;
	rpl::variable<bool> _vaultSavePrivate = true;
	rpl::variable<bool> _vaultSaveGroups = true;
	rpl::variable<bool> _vaultSaveChannels = true;
	rpl::variable<bool> _vaultSaveBots = true;
	rpl::variable<bool> _vaultSaveArchived = true;
	rpl::variable<bool> _vaultSavePhotos = true;
	rpl::variable<bool> _vaultSaveVideos = true;
	rpl::variable<bool> _vaultSaveVoice = true;
	rpl::variable<bool> _vaultSaveFiles = true;
	rpl::variable<int> _vaultMaxFileMb = 0;
	QStringList _vaultProtectedChats;

	bool _loaded = false;
};

} // namespace Wicked
