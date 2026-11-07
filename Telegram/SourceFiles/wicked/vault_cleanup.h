#pragma once

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QRegularExpression>
#include <QtCore/QSet>
#include <QtCore/QStringList>
#include <algorithm>

namespace Wicked::VaultCleanup {

struct Result {
	int deletedCount = 0;
	qint64 freedBytes = 0;
};

[[nodiscard]] inline Result Run(
		const QString &directory,
		int ttlDays,
		qint64 quotaBytes,
		const QStringList &protectedChats,
		const QDateTime &now) {
	auto result = Result();
	if (ttlDays <= 0 && quotaBytes <= 0) {
		return result;
	}
	const auto managed = QRegularExpression(
		QString::fromLatin1("^[0-9]+_[0-9]+_.+"));
	const auto files = QDir(directory).entryInfoList(
		QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
	auto candidates = QFileInfoList();
	auto protectedKeys = QSet<QString>();
	auto protectedPeers = QSet<QString>();
	for (const auto &key : protectedChats) {
		const auto peer = key.section(':', 1, 1);
		if (!peer.isEmpty()) {
			protectedKeys.insert(key);
			protectedPeers.insert(peer);
		}
	}
	auto total = qint64(0);
	for (const auto &file : files) {
		if (!managed.match(file.fileName()).hasMatch()) {
			continue;
		}
		const auto first = file.fileName().section('_', 0, 0);
		const auto second = file.fileName().section('_', 1, 1);
		const auto protectedFile = protectedKeys.contains(first + ':' + second)
			|| protectedPeers.contains(first);
		const auto size = file.size();
		const auto expired = ttlDays > 0
			&& file.lastModified().secsTo(now) > qint64(ttlDays) * 86400;
		if (!protectedFile && expired && QFile::remove(file.absoluteFilePath())) {
			++result.deletedCount;
			result.freedBytes += size;
			continue;
		}
		total += size;
		if (!protectedFile) {
			candidates.push_back(file);
		}
	}
	std::sort(candidates.begin(), candidates.end(), [](const auto &a, const auto &b) {
		return a.lastModified() < b.lastModified();
	});
	for (const auto &file : candidates) {
		if (quotaBytes <= 0 || total <= quotaBytes) {
			break;
		}
		const auto size = file.size();
		if (QFile::remove(file.absoluteFilePath())) {
			total -= size;
			++result.deletedCount;
			result.freedBytes += size;
		}
	}
	return result;
}

} // namespace Wicked::VaultCleanup
