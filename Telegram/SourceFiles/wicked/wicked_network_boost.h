/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#pragma once

#include "wicked/wicked_config.h"
#include <crl/crl_time.h>
#include <QtCore/QString>

namespace Wicked {

class NetworkBoost final {
public:
	// MTProto download optimization
	[[nodiscard]] static int InitialDownloadSessionsCount();
	[[nodiscard]] static int MaxDownloadSessionsCount();
	[[nodiscard]] static int InitialDownloadWaitedAmount();
	[[nodiscard]] static int MaxDownloadWaitedAmount();

	// MTProto upload optimization
	[[nodiscard]] static int TargetUploadSessionsCount(int64 fileSize);
	[[nodiscard]] static int MaxUploadPerSession(int64 fileSize);
	[[nodiscard]] static crl::time UploadRequestInterval(int64 fileSize);

	// Traffic & speed tracking
	static void TrackBytes(bool isUpload, int bytes);
	[[nodiscard]] static float DownloadSpeedMBps();
	[[nodiscard]] static float UploadSpeedMBps();
	[[nodiscard]] static QString FormatSpeed(float mbps);

	static void ResetMetrics();
};

} // namespace Wicked
