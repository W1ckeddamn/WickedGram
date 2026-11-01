/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#include "wicked/wicked_network_boost.h"
#include "storage/download_manager_mtproto.h"

#include <mutex>
#include <algorithm>

namespace Wicked {
namespace {

constexpr auto kOffInitialSessions = 1;
constexpr auto kFastInitialSessions = 4;
constexpr auto kUltraInitialSessions = 8;
constexpr auto kMaxDownloadSessions = 8;

constexpr auto kOffInitialWaited = 4 * Storage::kDownloadPartSize;
constexpr auto kFastInitialWaited = 16 * Storage::kDownloadPartSize;
constexpr auto kUltraInitialWaited = 32 * Storage::kDownloadPartSize;

constexpr auto kOffMaxWaited = 16 * Storage::kDownloadPartSize;
constexpr auto kFastMaxWaited = 32 * Storage::kDownloadPartSize;
constexpr auto kUltraMaxWaited = 64 * Storage::kDownloadPartSize;

struct TrafficTracker {
	std::mutex mutex;
	int64 downloadAccumulated = 0;
	crl::time lastDownloadTime = 0;
	float currentDownloadSpeed = 0.0f;

	int64 uploadAccumulated = 0;
	crl::time lastUploadTime = 0;
	float currentUploadSpeed = 0.0f;
};

TrafficTracker &Tracker() {
	static TrafficTracker tracker;
	return tracker;
}

} // namespace

int NetworkBoost::InitialDownloadSessionsCount() {
	switch (Config::DownloadBoost()) {
	case NetworkBoostMode::Ultra:
		return kUltraInitialSessions;
	case NetworkBoostMode::Fast:
		return kFastInitialSessions;
	case NetworkBoostMode::Off:
	default:
		return kOffInitialSessions;
	}
}

int NetworkBoost::MaxDownloadSessionsCount() {
	return kMaxDownloadSessions;
}

int NetworkBoost::InitialDownloadWaitedAmount() {
	switch (Config::DownloadBoost()) {
	case NetworkBoostMode::Ultra:
		return kUltraInitialWaited;
	case NetworkBoostMode::Fast:
		return kFastInitialWaited;
	case NetworkBoostMode::Off:
	default:
		return kOffInitialWaited;
	}
}

int NetworkBoost::MaxDownloadWaitedAmount() {
	switch (Config::DownloadBoost()) {
	case NetworkBoostMode::Ultra:
		return kUltraMaxWaited;
	case NetworkBoostMode::Fast:
		return kFastMaxWaited;
	case NetworkBoostMode::Off:
	default:
		return kOffMaxWaited;
	}
}

int NetworkBoost::TargetUploadSessionsCount(int64 fileSize) {
	const auto mode = Config::UploadBoost();
	if (mode == NetworkBoostMode::Off) {
		return 4;
	}

	if (Config::UploadBoostFileSizeAware()) {
		if (fileSize > 0 && fileSize < 10 * 1024 * 1024LL) {
			return 4;
		}
		if (fileSize >= 100 * 1024 * 1024LL && mode == NetworkBoostMode::Ultra) {
			return 8;
		}
		return (mode == NetworkBoostMode::Ultra) ? 8 : 6;
	}

	return (mode == NetworkBoostMode::Ultra) ? 8 : 6;
}

int NetworkBoost::MaxUploadPerSession(int64 fileSize) {
	const auto mode = Config::UploadBoost();
	if (mode == NetworkBoostMode::Ultra) {
		return 4 * 1024 * 1024;
	} else if (mode == NetworkBoostMode::Fast) {
		return 2 * 1024 * 1024;
	}
	return 1024 * 1024;
}

crl::time NetworkBoost::UploadRequestInterval(int64 fileSize) {
	const auto mode = Config::UploadBoost();
	if (mode == NetworkBoostMode::Ultra) {
		return crl::time(60);
	} else if (mode == NetworkBoostMode::Fast) {
		return crl::time(120);
	}
	return crl::time(250);
}

void NetworkBoost::TrackBytes(bool isUpload, int bytes) {
	if (bytes <= 0) {
		return;
	}
	const auto now = crl::now();
	auto &t = Tracker();
	std::lock_guard lock(t.mutex);

	if (isUpload) {
		t.uploadAccumulated += bytes;
		if (!t.lastUploadTime) {
			t.lastUploadTime = now;
		} else if (now - t.lastUploadTime >= 1000) {
			const auto elapsedSec = std::max(0.1f, float(now - t.lastUploadTime) / 1000.0f);
			t.currentUploadSpeed = (float(t.uploadAccumulated) / (1024.0f * 1024.0f)) / elapsedSec;
			t.uploadAccumulated = 0;
			t.lastUploadTime = now;
		}
	} else {
		t.downloadAccumulated += bytes;
		if (!t.lastDownloadTime) {
			t.lastDownloadTime = now;
		} else if (now - t.lastDownloadTime >= 1000) {
			const auto elapsedSec = std::max(0.1f, float(now - t.lastDownloadTime) / 1000.0f);
			t.currentDownloadSpeed = (float(t.downloadAccumulated) / (1024.0f * 1024.0f)) / elapsedSec;
			t.downloadAccumulated = 0;
			t.lastDownloadTime = now;
		}
	}
}

float NetworkBoost::DownloadSpeedMBps() {
	auto &t = Tracker();
	std::lock_guard lock(t.mutex);
	if (t.lastDownloadTime && crl::now() - t.lastDownloadTime > 2500) {
		t.currentDownloadSpeed = 0.0f;
	}
	return t.currentDownloadSpeed;
}

float NetworkBoost::UploadSpeedMBps() {
	auto &t = Tracker();
	std::lock_guard lock(t.mutex);
	if (t.lastUploadTime && crl::now() - t.lastUploadTime > 2500) {
		t.currentUploadSpeed = 0.0f;
	}
	return t.currentUploadSpeed;
}

QString NetworkBoost::FormatSpeed(float mbps) {
	if (mbps < 0.01f) {
		return u"0.00 MB/s"_q;
	}
	return QString::number(mbps, 'f', 2) + u" MB/s"_q;
}

void NetworkBoost::ResetMetrics() {
	auto &t = Tracker();
	std::lock_guard lock(t.mutex);
	t.downloadAccumulated = 0;
	t.lastDownloadTime = 0;
	t.currentDownloadSpeed = 0.0f;
	t.uploadAccumulated = 0;
	t.lastUploadTime = 0;
	t.currentUploadSpeed = 0.0f;
}

} // namespace Wicked
