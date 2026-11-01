/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#pragma once

#include "base/basic_types.h"
#include <QtCore/QString>
#include <base/flat_map.h>

namespace Wicked {

class RealNameCache final {
public:
	static RealNameCache &Instance();

	void putStranger(uint64 userId, const QString &fullName);
	[[nodiscard]] QString getOriginalName(uint64 userId);

	void load();
	void save();

private:
	RealNameCache();
	~RealNameCache() = default;

	RealNameCache(const RealNameCache &) = delete;
	RealNameCache &operator=(const RealNameCache &) = delete;

	static QString CachePath();

	base::flat_map<uint64, QString> _cache;
	bool _loaded = false;
	bool _dirty = false;
};

} // namespace Wicked
