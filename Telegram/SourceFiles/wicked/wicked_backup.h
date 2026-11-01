/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#pragma once

#include <QtCore/QString>

namespace Wicked {

class BackupManager final {
public:
	[[nodiscard]] static QString DefaultBackupDirectory();
	[[nodiscard]] static QString ExportBackup(const QString &targetFilePath = QString());
	static bool ImportBackup(const QString &filePath);
};

} // namespace Wicked
