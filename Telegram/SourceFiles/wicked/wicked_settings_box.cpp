/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#include "wicked/wicked_settings_box.h"

#include "wicked/wicked_config.h"
#include "lang/lang_keys.h"
#include "ui/widgets/checkbox.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/buttons.h"
#include "ui/layers/generic_box.h"
#include "styles/style_boxes.h"
#include "styles/style_settings.h"

namespace Wicked {

void SettingsBox(not_null<Ui::GenericBox*> box) {
	box->setTitle(rpl::single(u"WickedGram Settings"_q));

	// Category: Privacy & Protection
	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Приватность и защита"_q,
			st::boxLabel),
		st::settingsCheckboxPadding);

	// Toggle: Bypass Content Protection
	const auto bypassCheckbox = box->addRow(
		object_ptr<Ui::Checkbox>(
			box,
			u"Обход защиты контента (Bypass Content Protection)"_q,
			Config::BypassContentProtection()),
		st::settingsCheckboxPadding);

	bypassCheckbox->checkedChanges(
	) | rpl::start_with_next([=](bool checked) {
		Config::SetBypassContentProtection(checked);
	}, box->lifetime());

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Разрешает копирование текста, пересылку сообщений и сохранение любых медиафайлов из каналов и чатов с ограниченным доступом."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	box->addButton(tr::lng_close(), [=] {
		box->closeBox();
	});
}

} // namespace Wicked
