/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#pragma once

#include <rpl/variable.h>
#include <rpl/producer.h>
#include <QtCore/QString>

namespace Wicked {

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

	[[nodiscard]] static QString FixSocialLinks(QString text);
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

	bool _loaded = false;
};

} // namespace Wicked
