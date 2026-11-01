/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#include "wicked/wicked_lyrics.h"

#include "window/window_session_controller.h"
#include "media/player/media_player_instance.h"
#include "lang/lang_keys.h"
#include "ui/layers/generic_box.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/buttons.h"
#include "ui/toast/toast.h"
#include "styles/style_boxes.h"
#include "styles/style_settings.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QUrl>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QStringList>
#include <QtGui/QGuiApplication>
#include <QtGui/QClipboard>
#include <algorithm>

namespace Wicked {

LyricsHelper &LyricsHelper::Instance() {
	static LyricsHelper instance;
	return instance;
}

std::vector<LyricsLine> LyricsHelper::ParseSyncedLines(const QString &lrcContent) {
	std::vector<LyricsLine> result;
	const auto lines = lrcContent.split('\n');
	for (auto line : lines) {
		line = line.trimmed();
		if (!line.startsWith('[') || !line.contains(']')) {
			continue;
		}
		const auto closeIdx = line.indexOf(']');
		const auto timeStr = line.mid(1, closeIdx - 1);
		const auto text = line.mid(closeIdx + 1).trimmed();
		if (!timeStr.contains(':')) {
			continue;
		}
		const auto colonIdx = timeStr.indexOf(':');
		bool okMin = false;
		const auto minutes = timeStr.left(colonIdx).toInt(&okMin);
		if (!okMin) continue;

		auto secPart = timeStr.mid(colonIdx + 1);
		secPart.replace(':', '.');
		const auto dotIdx = secPart.indexOf('.');
		int seconds = 0;
		int ms = 0;
		if (dotIdx >= 0) {
			seconds = secPart.left(dotIdx).toInt();
			const auto msStr = secPart.mid(dotIdx + 1);
			if (msStr.size() == 1) {
				ms = msStr.toInt() * 100;
			} else if (msStr.size() == 2) {
				ms = msStr.toInt() * 10;
			} else {
				ms = msStr.left(3).toInt();
			}
		} else {
			seconds = secPart.toInt();
		}

		const auto totalMs = int64(minutes * 60 + seconds) * 1000 + ms;
		result.push_back(LyricsLine{ totalMs, text });
	}
	std::sort(result.begin(), result.end(), [](const LyricsLine &a, const LyricsLine &b) {
		return a.timeMs < b.timeMs;
	});
	return result;
}

LyricsResult LyricsHelper::ParseLrclibJson(
		const QJsonObject &obj,
		const QString &fallbackTitle,
		const QString &fallbackArtist) {
	LyricsResult res;
	res.title = obj.value("trackName").toString(fallbackTitle);
	res.artist = obj.value("artistName").toString(fallbackArtist);
	const auto synced = obj.value("syncedLyrics").toString();
	const auto plain = obj.value("plainLyrics").toString();

	if (!synced.isEmpty()) {
		res.lines = ParseSyncedLines(synced);
		if (!res.lines.empty()) {
			res.isSynced = true;
			res.plainText = plain;
			res.source = u"LRCLIB (Синхронизировано)"_q;
			return res;
		}
	}

	if (!plain.isEmpty()) {
		res.isSynced = false;
		res.plainText = plain;
		res.source = u"LRCLIB"_q;
		return res;
	}

	return res;
}

void LyricsHelper::fetchLyrics(
		const QString &title,
		const QString &artist,
		int durationSecs,
		Fn<void(const LyricsResult &)> callback) {
	const auto cacheKey = (artist.trimmed().toLower() + u" - "_q + title.trimmed().toLower());
	if (const auto it = _cache.find(cacheKey); it != _cache.end()) {
		callback(it->second);
		return;
	}

	auto urlStr = QString(u"https://lrclib.net/api/get?track_name=%1"_q)
		.arg(QString::fromUtf8(QUrl::toPercentEncoding(title.trimmed())));
	if (!artist.trimmed().isEmpty()) {
		urlStr += u"&artist_name="_q + QString::fromUtf8(QUrl::toPercentEncoding(artist.trimmed()));
	}
	if (durationSecs > 0) {
		urlStr += u"&duration="_q + QString::number(durationSecs);
	}

	auto request = QNetworkRequest(QUrl(urlStr));
	request.setRawHeader("User-Agent", "WickedGram-Desktop/1.0");

	auto *manager = new QNetworkAccessManager();
	auto *reply = manager->get(request);

	QObject::connect(reply, &QNetworkReply::finished, [=]() {
		const auto replyGuard = gsl::finally([=] {
			reply->deleteLater();
			manager->deleteLater();
		});

		if (reply->error() == QNetworkReply::NoError) {
			const auto data = reply->readAll();
			const auto doc = QJsonDocument::fromJson(data);
			if (doc.isObject()) {
				const auto res = ParseLrclibJson(doc.object(), title, artist);
				if (res.hasLyrics()) {
					_cache[cacheKey] = res;
					callback(res);
					return;
				}
			}
		}

		// Fallback: search query /api/search?q=...
		const auto searchQuery = (!artist.trimmed().isEmpty() ? (artist.trimmed() + u" "_q) : QString()) + title.trimmed();
		const auto searchUrl = QString(u"https://lrclib.net/api/search?q=%1"_q)
			.arg(QString::fromUtf8(QUrl::toPercentEncoding(searchQuery)));
		auto searchReq = QNetworkRequest(QUrl(searchUrl));
		searchReq.setRawHeader("User-Agent", "WickedGram-Desktop/1.0");

		auto *searchManager = new QNetworkAccessManager();
		auto *searchReply = searchManager->get(searchReq);

		QObject::connect(searchReply, &QNetworkReply::finished, [=]() {
			const auto searchGuard = gsl::finally([=] {
				searchReply->deleteLater();
				searchManager->deleteLater();
			});

			if (searchReply->error() == QNetworkReply::NoError) {
				const auto sData = searchReply->readAll();
				const auto sDoc = QJsonDocument::fromJson(sData);
				if (sDoc.isArray()) {
					const auto arr = sDoc.array();
					if (!arr.isEmpty() && arr.first().isObject()) {
						const auto res = ParseLrclibJson(arr.first().toObject(), title, artist);
						if (res.hasLyrics()) {
							_cache[cacheKey] = res;
							callback(res);
							return;
						}
					}
				}
			}

			LyricsResult emptyRes;
			emptyRes.title = title;
			emptyRes.artist = artist;
			callback(emptyRes);
		});
	});
}

void LyricsBox(
		not_null<Ui::GenericBox*> box,
		QString title,
		QString artist,
		int durationSecs) {
	const auto displayTitle = artist.isEmpty() ? title : (artist + u" — "_q + title);
	box->setTitle(rpl::single(displayTitle));

	const auto statusLabel = box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Загрузка текста песни из LRCLIB..."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	const auto contentLabel = box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			QString(),
			st::boxLabel),
		st::settingsCheckboxPadding);

	LyricsHelper::Instance().fetchLyrics(title, artist, durationSecs, [=](const LyricsResult &result) {
		if (!result.hasLyrics()) {
			statusLabel->setText(u"Текст песни не найден в базе данных LRCLIB."_q);
			return;
		}

		statusLabel->setText(result.source + (result.isSynced
			? (u" ("_q + QString::number(result.lines.size()) + u" строк с таймкодами)"_q)
			: u" (Обычный текст)"_q));

		contentLabel->setText(result.getFullText());

		const auto copyBtn = box->addRow(
			object_ptr<Ui::RoundButton>(
				box,
				rpl::single(u"Копировать текст песни"_q),
				st::defaultBoxButton),
			st::settingsCheckboxPadding);

		copyBtn->setClickedCallback([=] {
			QGuiApplication::clipboard()->setText(result.getFullText());
			Ui::Toast::Show(u"Текст песни скопирован в буфер обмена"_q);
		});
	});

	box->addButton(tr::lng_close(), [=] {
		box->closeBox();
	});
}

void ShowLyricsBox(
		not_null<Window::SessionController*> controller,
		const QString &title,
		const QString &artist,
		int durationSecs) {
	controller->show(Box(LyricsBox, title, artist, durationSecs));
}

} // namespace Wicked
