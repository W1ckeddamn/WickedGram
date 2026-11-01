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
	if (obj.contains("socialLinkFixer")) {
		_socialLinkFixer = obj.value("socialLinkFixer").toBool(true);
	}
	if (obj.contains("showSecondsInTime")) {
		_showSecondsInTime = obj.value("showSecondsInTime").toBool(false);
	}
	if (obj.contains("customBadgesEnabled")) {
		_customBadgesEnabled = obj.value("customBadgesEnabled").toBool(true);
	}
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
	obj.insert("socialLinkFixer", _socialLinkFixer.current());
	obj.insert("showSecondsInTime", _showSecondsInTime.current());
	obj.insert("customBadgesEnabled", _customBadgesEnabled.current());

	QFile file(path);
	if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
	}
}

} // namespace Wicked
