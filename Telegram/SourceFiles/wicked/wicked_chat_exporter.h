/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#pragma once

#include "base/basic_types.h"
#include <QtCore/QString>
#include <vector>

namespace Wicked {

enum class ExportFormat {
	Html,
	Markdown,
	Json,
	PlainText,
};

struct ExportMessage {
	uint64 id = 0;
	int64 date = 0;
	QString author;
	QString text;
	QString mediaDesc;
	bool isOut = false;
	bool isDeleted = false;
	bool isEdited = false;
	QString previousText;
};

class ChatExporter final {
public:
	[[nodiscard]] static QString DefaultExportDirectory();

	[[nodiscard]] static QString FormatHtml(
		const QString &chatTitle,
		uint64 peerId,
		const std::vector<ExportMessage> &messages);

	[[nodiscard]] static QString FormatMarkdown(
		const QString &chatTitle,
		uint64 peerId,
		const std::vector<ExportMessage> &messages);

	[[nodiscard]] static QString FormatJson(
		const QString &chatTitle,
		uint64 peerId,
		const std::vector<ExportMessage> &messages);

	[[nodiscard]] static QString FormatPlainText(
		const QString &chatTitle,
		uint64 peerId,
		const std::vector<ExportMessage> &messages);

	static QString ExportToFile(
		const QString &chatTitle,
		uint64 peerId,
		const std::vector<ExportMessage> &messages,
		ExportFormat format,
		const QString &targetFilePath = QString());
};

} // namespace Wicked
