/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#include "wicked/wicked_chat_exporter.h"

#include "storage/localstorage.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDateTime>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>

namespace Wicked {
namespace {

QString EscapeHtml(const QString &text) {
	auto copy = text;
	copy.replace('&', u"&amp;"_q);
	copy.replace('<', u"&lt;"_q);
	copy.replace('>', u"&gt;"_q);
	copy.replace('"', u"&quot;"_q);
	return copy;
}

} // namespace

QString ChatExporter::DefaultExportDirectory() {
	const auto dir = cWorkingDir() + u"tdata/wickedgram_exports/"_q;
	QDir().mkpath(dir);
	return dir;
}

QString ChatExporter::FormatHtml(
		const QString &chatTitle,
		uint64 peerId,
		const std::vector<ExportMessage> &messages) {
	QString out;
	out.reserve(messages.size() * 300 + 1024);

	out.append(u"<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"UTF-8\">\n"_q);
	out.append(u"<title>WickedGram Export - "_q + EscapeHtml(chatTitle) + u"</title>\n"_q);
	out.append(u"<style>\n"_q);
	out.append(u"body { background: #0e1621; color: #f5f5f5; font-family: -apple-system, Segoe UI, Roboto, Helvetica, sans-serif; margin: 0; padding: 20px; }\n"_q);
	out.append(u".container { max-width: 800px; margin: 0 auto; }\n"_q);
	out.append(u".header { background: #17212b; padding: 20px; border-radius: 12px; margin-bottom: 20px; border: 1px solid #2b394a; }\n"_q);
	out.append(u".header h1 { margin: 0 0 8px 0; font-size: 22px; color: #5288c1; }\n"_q);
	out.append(u".header p { margin: 4px 0; font-size: 13px; color: #7f91a4; }\n"_q);
	out.append(u".msg { margin-bottom: 12px; display: flex; flex-direction: column; }\n"_q);
	out.append(u".msg-bubble { background: #182533; border-radius: 10px; padding: 10px 14px; max-width: 85%; align-self: flex-start; border: 1px solid #242f3d; word-wrap: break-word; }\n"_q);
	out.append(u".msg-out { align-self: flex-end; background: #2b5278; border-color: #3b6b98; }\n"_q);
	out.append(u".msg-author { font-size: 13px; font-weight: bold; color: #64b5f6; margin-bottom: 4px; }\n"_q);
	out.append(u".msg-text { font-size: 14px; line-height: 1.45; white-space: pre-wrap; }\n"_q);
	out.append(u".msg-media { font-size: 13px; color: #40c4ff; margin: 4px 0; font-style: italic; }\n"_q);
	out.append(u".msg-meta { font-size: 11px; color: #7f91a4; margin-top: 6px; display: flex; align-items: center; justify-content: flex-end; gap: 8px; }\n"_q);
	out.append(u".badge-del { background: #e53935; color: #fff; padding: 2px 6px; border-radius: 4px; font-size: 10px; font-weight: bold; }\n"_q);
	out.append(u".badge-edit { background: #ffa726; color: #000; padding: 2px 6px; border-radius: 4px; font-size: 10px; font-weight: bold; }\n"_q);
	out.append(u".edits-details { margin-top: 6px; font-size: 12px; color: #8ab4f8; background: rgba(0,0,0,0.2); padding: 6px 10px; border-radius: 6px; }\n"_q);
	out.append(u"</style>\n</head>\n<body>\n<div class=\"container\">\n"_q);

	out.append(u"<div class=\"header\">\n"_q);
	out.append(u"<h1>"_q + EscapeHtml(chatTitle) + u"</h1>\n"_q);
	out.append(u"<p>Peer ID: "_q + QString::number(peerId) + u"</p>\n"_q);
	out.append(u"<p>Export Date: "_q + QDateTime::currentDateTime().toString(u"yyyy-MM-dd HH:mm:ss"_q) + u"</p>\n"_q);
	out.append(u"<p>Total Messages: "_q + QString::number(messages.size()) + u"</p>\n"_q);
	out.append(u"</div>\n"_q);

	for (const auto &msg : messages) {
		const auto dateStr = QDateTime::fromSecsSinceEpoch(msg.date).toString(u"yyyy-MM-dd HH:mm:ss"_q);
		out.append(u"<div class=\"msg\">\n"_q);
		out.append(QString(u"  <div class=\"msg-bubble%1\">\n"_q).arg(msg.isOut ? u" msg-out"_q : u""_q));
		out.append(u"    <div class=\"msg-author\">"_q + EscapeHtml(msg.author.isEmpty() ? u"Unknown"_q : msg.author) + u"</div>\n"_q);
		if (!msg.mediaDesc.isEmpty()) {
			out.append(u"    <div class=\"msg-media\">"_q + EscapeHtml(msg.mediaDesc) + u"</div>\n"_q);
		}
		if (!msg.text.isEmpty()) {
			out.append(u"    <div class=\"msg-text\">"_q + EscapeHtml(msg.text) + u"</div>\n"_q);
		}
		if (msg.isEdited && !msg.previousText.isEmpty()) {
			out.append(u"    <div class=\"edits-details\">Было: "_q + EscapeHtml(msg.previousText) + u"</div>\n"_q);
		}
		out.append(u"    <div class=\"msg-meta\">\n"_q);
		if (msg.isDeleted) {
			out.append(u"      <span class=\"badge-del\">Deleted</span>\n"_q);
		}
		if (msg.isEdited) {
			out.append(u"      <span class=\"badge-edit\">Edited</span>\n"_q);
		}
		out.append(u"      <span>"_q + dateStr + u" (ID: "_q + QString::number(msg.id) + u")</span>\n"_q);
		out.append(u"    </div>\n"_q);
		out.append(u"  </div>\n</div>\n"_q);
	}

	out.append(u"</div>\n</body>\n</html>\n"_q);
	return out;
}

QString ChatExporter::FormatMarkdown(
		const QString &chatTitle,
		uint64 peerId,
		const std::vector<ExportMessage> &messages) {
	QString out;
	out.append(u"# WickedGram Export: "_q + chatTitle + u" (ID: "_q + QString::number(peerId) + u")\n\n"_q);
	out.append(u"Export Date: "_q + QDateTime::currentDateTime().toString(u"yyyy-MM-dd HH:mm:ss"_q) + u"\n"_q);
	out.append(u"Total Messages: "_q + QString::number(messages.size()) + u"\n\n---\n\n"_q);

	for (const auto &msg : messages) {
		const auto dateStr = QDateTime::fromSecsSinceEpoch(msg.date).toString(u"yyyy-MM-dd HH:mm:ss"_q);
		auto tags = QString();
		if (msg.isDeleted) tags += u" [🗑 УДАЛЕНО]"_q;
		if (msg.isEdited) tags += u" [✏️ ИЗМЕНЕНО]"_q;

		out.append(u"### **"_q + (msg.author.isEmpty() ? u"User"_q : msg.author) + u"** (" + dateStr + u", ID: " + QString::number(msg.id) + u")" + tags + u"\n"_q);
		if (!msg.mediaDesc.isEmpty()) {
			out.append(u"> *" + msg.mediaDesc + u"*\n\n"_q);
		}
		if (!msg.text.isEmpty()) {
			out.append(msg.text + u"\n\n"_q);
		}
		if (msg.isEdited && !msg.previousText.isEmpty()) {
			out.append(u"*Предыдущая редакция:* " + msg.previousText + u"\n\n"_q);
		}
		out.append(u"---\n\n"_q);
	}
	return out;
}

QString ChatExporter::FormatJson(
		const QString &chatTitle,
		uint64 peerId,
		const std::vector<ExportMessage> &messages) {
	QJsonObject root;
	root.insert("chat_title", chatTitle);
	root.insert("peer_id", qint64(peerId));
	root.insert("export_date", QDateTime::currentDateTime().toString(Qt::ISODate));
	root.insert("total_messages", int(messages.size()));

	QJsonArray arr;
	for (const auto &msg : messages) {
		QJsonObject item;
		item.insert("id", qint64(msg.id));
		item.insert("date", qint64(msg.date));
		item.insert("author", msg.author);
		item.insert("is_out", msg.isOut);
		item.insert("text", msg.text);
		if (!msg.mediaDesc.isEmpty()) {
			item.insert("media", msg.mediaDesc);
		}
		item.insert("is_deleted", msg.isDeleted);
		item.insert("is_edited", msg.isEdited);
		if (msg.isEdited && !msg.previousText.isEmpty()) {
			item.insert("previous_text", msg.previousText);
		}
		arr.append(item);
	}
	root.insert("messages", arr);
	return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

QString ChatExporter::FormatPlainText(
		const QString &chatTitle,
		uint64 peerId,
		const std::vector<ExportMessage> &messages) {
	QString out;
	out.append(u"WickedGram Chat Export: "_q + chatTitle + u" (" + QString::number(peerId) + u")\n\n"_q);
	for (const auto &msg : messages) {
		const auto dateStr = QDateTime::fromSecsSinceEpoch(msg.date).toString(u"yyyy-MM-dd HH:mm:ss"_q);
		auto tags = QString();
		if (msg.isDeleted) tags += u" [DELETED]"_q;
		if (msg.isEdited) tags += u" [EDITED]"_q;

		out.append(QString(u"[%1] %2%3: %4\n"_q)
			.arg(dateStr)
			.arg(msg.author.isEmpty() ? u"User"_q : msg.author)
			.arg(tags)
			.arg(msg.text));
	}
	return out;
}

QString ChatExporter::ExportToFile(
		const QString &chatTitle,
		uint64 peerId,
		const std::vector<ExportMessage> &messages,
		ExportFormat format,
		const QString &targetFilePath) {
	auto dest = targetFilePath;
	if (dest.isEmpty()) {
		const auto timeStr = QDateTime::currentDateTime().toString(u"yyyyMMdd_HHmmss"_q);
		auto ext = u".html"_q;
		if (format == ExportFormat::Markdown) ext = u".md"_q;
		else if (format == ExportFormat::Json) ext = u".json"_q;
		else if (format == ExportFormat::PlainText) ext = u".txt"_q;

		dest = DefaultExportDirectory() + u"chat_"_q + QString::number(peerId) + u"_"_q + timeStr + ext;
	}

	QString content;
	switch (format) {
	case ExportFormat::Html: content = FormatHtml(chatTitle, peerId, messages); break;
	case ExportFormat::Markdown: content = FormatMarkdown(chatTitle, peerId, messages); break;
	case ExportFormat::Json: content = FormatJson(chatTitle, peerId, messages); break;
	case ExportFormat::PlainText: content = FormatPlainText(chatTitle, peerId, messages); break;
	}

	const auto dir = QFileInfo(dest).dir();
	if (!dir.exists()) {
		dir.mkpath(u"."_q);
	}

	QFile file(dest);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		return QString();
	}
	file.write(content.toUtf8());
	return dest;
}

} // namespace Wicked
