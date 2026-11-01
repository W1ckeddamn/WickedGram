/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#include "wicked/wicked_vault.h"

#include "wicked/wicked_config.h"
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
	if (sourcePath.isEmpty() || !QFile::exists(sourcePath) || destPath.isEmpty()) {
		return false;
	}
	if (QFile::exists(destPath)) {
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

	return QFile::copy(sourcePath, destPath);
}

bool Vault::IsolateMedia(not_null<HistoryItem*> item) {
	const auto media = item->media();
	if (!media) {
		return false;
	}

	const auto vaultDir = VaultDirectory();
	const auto peerId = item->history()->peer->id.value;
	const auto msgId = item->id.bare;

	if (const auto document = media->document()) {
		const auto localPath = document->filepath(true);
		const auto fileName = document->filename().isEmpty()
			? (QString::number(document->id) + u".bin"_q)
			: document->filename();
		const auto destName = QString::number(peerId)
			+ u"_"_q + QString::number(msgId)
			+ u"_"_q + fileName;
		const auto destPath = vaultDir + destName;

		if (!localPath.isEmpty() && QFile::exists(localPath)) {
			return LinkOrCopyFile(localPath, destPath);
		} else if (const auto view = document->activeMediaView()) {
			const auto bytes = view->bytes();
			if (!bytes.isEmpty()) {
				QFile file(destPath);
				if (file.open(QIODevice::WriteOnly)) {
					file.write(bytes);
					return true;
				}
			}
		}
	} else if (const auto photo = media->photo()) {
		const auto destName = QString::number(peerId)
			+ u"_"_q + QString::number(msgId)
			+ u"_photo.jpg"_q;
		const auto destPath = vaultDir + destName;
		if (QFile::exists(destPath)) {
			return true;
		}
		if (const auto view = photo->activeMediaView()) {
			return view->saveToFile(destPath);
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
	RotationResult result;
	if (ttlDays <= 0 && maxSizeGb <= 0) {
		return result;
	}
	const auto dirPath = VaultDirectory();
	QDir dir(dirPath);
	auto entries = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
	const auto now = QDateTime::currentDateTime();

	const auto maxAgeSecs = (ttlDays > 0) ? (int64(ttlDays) * 86400) : 0;
	QFileInfoList surviving;
	surviving.reserve(entries.size());

	for (const auto &info : entries) {
		if (info.size() == 0) {
			QFile::remove(info.absoluteFilePath());
			continue;
		}
		if (maxAgeSecs > 0) {
			const auto ageSecs = info.lastModified().secsTo(now);
			if (ageSecs > maxAgeSecs) {
				const auto size = info.size();
				if (QFile::remove(info.absoluteFilePath())) {
					result.deletedCount++;
					result.freedBytes += size;
					continue;
				}
			}
		}
	surviving.push_back(info);
	}

	if (maxSizeGb > 0) {
		const auto maxBytes = int64(maxSizeGb) * 1024 * 1024 * 1024;
		int64 currentBytes = 0;
		for (const auto &info : surviving) {
			currentBytes += info.size();
		}
		if (currentBytes > maxBytes) {
			std::sort(surviving.begin(), surviving.end(), [](const QFileInfo &a, const QFileInfo &b) {
				return a.lastModified() < b.lastModified();
			});
			for (const auto &info : surviving) {
				if (currentBytes <= maxBytes) {
					break;
				}
				const auto size = info.size();
				if (QFile::remove(info.absoluteFilePath())) {
					currentBytes -= size;
					result.deletedCount++;
					result.freedBytes += size;
				}
			}
		}
	}
	return result;
}

} // namespace Wicked
