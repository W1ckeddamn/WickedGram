/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#pragma once

#include "base/basic_types.h"
#include <QtCore/QString>
#include <QtCore/QJsonObject>
#include <vector>
#include <map>

namespace Window {
class SessionController;
} // namespace Window

namespace Wicked {

struct LyricsLine {
	int64 timeMs = 0;
	QString text;
};

struct LyricsResult {
	bool isSynced = false;
	std::vector<LyricsLine> lines;
	QString plainText;
	QString title;
	QString artist;
	QString source;

	[[nodiscard]] bool hasLyrics() const {
		return (isSynced && !lines.empty()) || !plainText.trimmed().isEmpty();
	}

	[[nodiscard]] int findCurrentLineIndex(int64 positionMs) const {
		if (!isSynced || lines.empty()) {
			return -1;
		}
		int active = -1;
		for (int i = 0; i < int(lines.size()); ++i) {
			if (lines[i].timeMs <= positionMs) {
				active = i;
			} else {
				break;
			}
		}
		return active;
	}

	[[nodiscard]] QString getFullText() const {
		if (!plainText.trimmed().isEmpty()) {
			return plainText.trimmed();
		}
		QStringList list;
		for (const auto &line : lines) {
			list.append(line.text);
		}
		return list.join('\n');
	}
};

class LyricsHelper final {
public:
	static LyricsHelper &Instance();

	void fetchLyrics(
		const QString &title,
		const QString &artist,
		int durationSecs,
		Fn<void(const LyricsResult &)> callback);

	[[nodiscard]] static std::vector<LyricsLine> ParseSyncedLines(const QString &lrcContent);
	[[nodiscard]] static LyricsResult ParseLrclibJson(
		const QJsonObject &obj,
		const QString &fallbackTitle,
		const QString &fallbackArtist);

private:
	LyricsHelper() = default;
	~LyricsHelper() = default;

	LyricsHelper(const LyricsHelper &) = delete;
	LyricsHelper &operator=(const LyricsHelper &) = delete;

	std::map<QString, LyricsResult> _cache;
};

void ShowLyricsBox(
	not_null<Window::SessionController*> controller,
	const QString &title,
	const QString &artist,
	int durationSecs);

} // namespace Wicked
