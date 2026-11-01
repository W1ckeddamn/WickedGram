/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#include "wicked/wicked_realname.h"

#include "storage/localstorage.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

namespace Wicked {

RealNameCache &RealNameCache::Instance() {
	static RealNameCache instance;
	return instance;
}

RealNameCache::RealNameCache() {
	load();
}

QString RealNameCache::CachePath() {
	return cWorkingDir() + u"tdata/wickedgram_realnames.json"_q;
}

void RealNameCache::putStranger(uint64 userId, const QString &fullName) {
	if (userId == 0 || fullName.trimmed().isEmpty()) {
		return;
	}
	load();
	if (_cache.contains(userId)) {
		return;
	}
	_cache.emplace(userId, fullName.trimmed());
	_dirty = true;
	save();
}

QString RealNameCache::getOriginalName(uint64 userId) {
	if (userId == 0) {
		return QString();
	}
	load();
	const auto it = _cache.find(userId);
	return (it != _cache.end()) ? it->second : QString();
}

void RealNameCache::load() {
	if (_loaded) {
		return;
	}
	_loaded = true;

	QFile file(CachePath());
	if (!file.open(QIODevice::ReadOnly)) {
		return;
	}

	const auto doc = QJsonDocument::fromJson(file.readAll());
	if (!doc.isObject()) {
		return;
	}
	const auto obj = doc.object();
	for (auto it = obj.begin(); it != obj.end(); ++it) {
		bool ok = false;
		const auto uid = it.key().toULongLong(&ok);
		if (ok && it.value().isString()) {
			_cache.emplace(uid, it.value().toString());
		}
	}
}

void RealNameCache::save() {
	if (!_dirty) {
		return;
	}
	_dirty = false;

	const auto path = CachePath();
	const auto dir = QFileInfo(path).dir();
	if (!dir.exists()) {
		dir.mkpath(u"."_q);
	}

	QJsonObject obj;
	for (const auto &[uid, name] : _cache) {
		obj.insert(QString::number(uid), name);
	}

	QFile file(path);
	if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		file.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
	}
}

} // namespace Wicked
