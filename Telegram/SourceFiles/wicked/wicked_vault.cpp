/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#include "wicked/wicked_vault.h"

#include "wicked/wicked_config.h"
#include "wicked/vault_cleanup.h"
#include "main/main_session.h"
#include "ui/image/image.h"
#include "history/history_item.h"
#include "history/history.h"
#include "data/data_peer.h"
#include "data/data_media_types.h"
#include "data/data_document.h"
#include "data/data_photo.h"
#include "data/data_photo_media.h"
#include "data/data_document_media.h"
#include "storage/localstorage.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDateTime>
#include <QtCore/QSaveFile>
#include <QtCore/QBuffer>
#include <algorithm>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace Wicked {

Vault &Vault::Instance() {
	static Vault instance;
	return instance;
}

QString Vault::VaultDirectory() {
	const auto dir = cWorkingDir() + u"tdata/wickedgram_vault/"_q;
	QDir().mkpath(dir);
	return dir;
}

bool Vault::LinkOrCopyFile(const QString &sourcePath, const QString &destPath) {
	if (sourcePath.isEmpty() || !QFile::exists(sourcePath) || destPath.isEmpty()
		|| QFileInfo(destPath).isSymLink()) {
		return false;
	}
	if (QFileInfo(destPath).isFile()
		&& QFileInfo(destPath).size() == QFileInfo(sourcePath).size()) {
		return true;
	}

#ifdef Q_OS_WIN
	if (CreateHardLinkW(
		reinterpret_cast<LPCWSTR>(destPath.utf16()),
		reinterpret_cast<LPCWSTR>(sourcePath.utf16()),
		nullptr)) {
		return true;
	}
#endif

	auto source = QFile(sourcePath);
	auto dest = QSaveFile(destPath);
	if (!source.open(QIODevice::ReadOnly) || !dest.open(QIODevice::WriteOnly)) {
		return false;
	}
	while (!source.atEnd()) {
		const auto bytes = source.read(256 * 1024);
		if (bytes.isEmpty() || dest.write(bytes) != bytes.size()) {
			return false;
		}
	}
	return dest.commit();
}

bool Vault::IsolateMedia(not_null<HistoryItem*> item) {
	const auto media = item->media();
	const auto history = item->history();
	const auto peer = history->peer;
	if (!media || item->id.bare <= 0 || item->out() || item->isEphemeral() || media->ttlSeconds() > 0
		|| (history->folder() && !Config::VaultSaveArchived())) {
		return false;
	}
	const auto allowed = peer->isBot()
		? Config::VaultSaveBots()
		: peer->isUser()
		? Config::VaultSavePrivate()
		: peer->isBroadcast()
		? Config::VaultSaveChannels()
		: Config::VaultSaveGroups();
	if (!allowed) {
		return false;
	}
	const auto maxBytes = qint64(Config::VaultMaxFileMb()) * 1024 * 1024;
	const auto prefix = QString::number(history->session().userId().bare)
		+ '_' + QString::number(peer->id.value)
		+ '_' + QString::number(item->id.bare) + '_';
	const auto finish = [&](bool saved) {
		static auto lastCleanup = qint64(0);
		const auto now = QDateTime::currentSecsSinceEpoch();
		if (saved && now - lastCleanup >= 60) {
			lastCleanup = now;
			const auto result = Rotate(Config::VaultTtlDays(), Config::VaultMaxSizeGb());
			if (result.deletedCount) {
				LOG(("WickedGram Vault: auto-cleaned %1 files").arg(result.deletedCount));
			}
		}
		return saved;
	};
	const auto writeBytes = [&](const QString &path, const QByteArray &bytes) {
		if (bytes.isEmpty() || (maxBytes && bytes.size() > maxBytes)) {
			return false;
		}
		if (QFile::exists(path)) {
			return true;
		}
		auto file = QSaveFile(path);
		return file.open(QIODevice::WriteOnly)
			&& file.write(bytes) == bytes.size()
			&& file.commit();
	};
	if (const auto document = media->document()) {
		const auto allowedType = (document->isVoiceMessage() || document->isVideoMessage())
			? Config::VaultSaveVoice()
			: (document->isVideoFile() || document->isAnimation())
			? Config::VaultSaveVideos()
			: Config::VaultSaveFiles();
		if (!allowedType || (maxBytes && document->size > maxBytes)) {
			return false;
		}
		auto name = document->filename();
		for (auto i = 0; i < name.size(); ++i) {
			if (name.at(i).unicode() < 32 || u"\\/:*?\"<>|"_q.contains(name.at(i))) {
				name[i] = '_';
			}
		}
		name = name.left(120);
		if (name.isEmpty()) {
			name = QString::number(document->id) + u".bin"_q;
		}
		const auto dest = VaultDirectory() + prefix
			+ QString::number(document->id) + '_' + name;
		const auto source = document->filepath(true);
		if (!source.isEmpty() && QFile::exists(source)) {
			if (maxBytes && QFileInfo(source).size() > maxBytes) {
				return false;
			}
			return finish(LinkOrCopyFile(source, dest));
		} else if (const auto view = document->activeMediaView()) {
			return finish(writeBytes(dest, view->bytes()));
		}
	} else if (const auto photo = media->photo()) {
		if (!Config::VaultSavePhotos()
			|| (maxBytes && photo->imageByteSize(Data::PhotoSize::Large) > maxBytes)) {
			return false;
		}
		if (const auto view = photo->activeMediaView()) {
			auto bytes = view->imageBytes(Data::PhotoSize::Large);
			if (bytes.isEmpty()) {
				const auto image = view->image(Data::PhotoSize::Large);
				if (!image) {
					return false;
				}
				auto buffer = QBuffer(&bytes);
				if (!buffer.open(QIODevice::WriteOnly)
					|| !image->original().save(&buffer, "JPG")) {
					return false;
				}
			}
			return finish(writeBytes(VaultDirectory() + prefix + u"photo.jpg"_q, bytes));
		}
	}
	return false;
}

Vault::Stats Vault::GetStats() {
	Stats stats;
	QDir dir(VaultDirectory());
	const auto entries = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
	stats.count = entries.size();
	for (const auto &info : entries) {
		stats.bytes += info.size();
	}
	return stats;
}

bool Vault::Clear() {
	QDir dir(VaultDirectory());
	const auto entries = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
	bool allOk = true;
	for (const auto &info : entries) {
		if (!QFile::remove(info.absoluteFilePath())) {
			allOk = false;
		}
	}
	return allOk;
}

Vault::RotationResult Vault::Rotate(int ttlDays, int maxSizeGb) {
	const auto result = VaultCleanup::Run(
		VaultDirectory(),
		ttlDays,
		qint64(maxSizeGb) * 1024 * 1024 * 1024,
		Config::VaultProtectedChats(),
		QDateTime::currentDateTime());
	return { result.deletedCount, result.freedBytes };
}

} // namespace Wicked
