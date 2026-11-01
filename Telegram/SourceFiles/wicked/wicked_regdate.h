/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#pragma once

#include "base/basic_types.h"
#include <QtCore/QString>

namespace Wicked {

class RegDate final {
public:
	[[nodiscard]] static int64 EstimateTimestamp(uint64 userId);
	[[nodiscard]] static QString FormatRegistrationDate(uint64 userId);
};

} // namespace Wicked
