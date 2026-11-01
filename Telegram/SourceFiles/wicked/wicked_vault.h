/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#pragma once

#include "base/basic_types.h"
#include <QtCore/QString>

class HistoryItem;

namespace Wicked {

class Vault final {
public:
	static Vault &Instance();

	[[nodiscard]] static QString VaultDirectory();
	static bool IsolateMedia(not_null<HistoryItem*> item);

	struct Stats {
		int count = 0;
		int64 bytes = 0;
	};
	[[nodiscard]] static Stats GetStats();
	static bool Clear();

private:
	Vault() = default;
	~Vault() = default;

	Vault(const Vault &) = delete;
	Vault &operator=(const Vault &) = delete;

	static bool LinkOrCopyFile(const QString &sourcePath, const QString &destPath);
};

} // namespace Wicked
