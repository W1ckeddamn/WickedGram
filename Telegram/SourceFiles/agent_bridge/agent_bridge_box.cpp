/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#include "agent_bridge/agent_bridge_box.h"

#include "agent_bridge/agent_settings.h"
#include "agent_bridge/agent_bridge.h"
#include "lang/lang_keys.h"
#include "ui/widgets/checkbox.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/buttons.h"
#include "ui/toast/toast.h"
#include "styles/style_boxes.h"
#include "styles/style_layers.h"
#include "styles/style_settings.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QClipboard>

namespace Agent {

void BridgeBox(not_null<Ui::GenericBox*> box) {
	box->setTitle(rpl::single(u"WickedGram AI Agent Bridge"_q));

	auto &settings = GlobalSettings();

	const auto enabledCheckbox = box->addRow(
		object_ptr<Ui::Checkbox>(
			box,
			u"Включить локальный мост управления ИИ-агентом"_q,
			settings.enabled()),
		st::settingsCheckboxPadding);

	enabledCheckbox->checkedChanges(
	) | rpl::on_next([=](bool checked) {
		GlobalSettings().setEnabled(checked);
	}, box->lifetime());

	const auto statusText = [=] {
		if (!GlobalSettings().enabled()) {
			return u"Статус: отключен в настройках"_q;
		} else if (Bridge::Instance().isRunning()) {
			return u"Статус: активен (http://127.0.0.1:"_q
				+ QString::number(Bridge::Instance().port())
				+ u")"_q;
		} else {
			return u"Статус: ошибка запуска сервера"_q;
		}
	};

	const auto statusLabel = box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			statusText(),
			st::boxLabel),
		st::settingsCheckboxPadding);

	const auto descLabel = box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Локальный REST/SSE API сервер позволяет внешним ИИ-агентам (Claude, AutoGPT, Python скриптам) читать сообщения, отправлять ответы и автоматизировать действия."_q,
			st::boxLabel),
		st::settingsCheckboxPadding);
	descLabel->setTextColorOverride(st::windowSubTextFg->c);

	const auto tokenTitle = box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Токен авторизации (Bearer):"_q,
			st::boxLabel),
		st::settingsCheckboxPadding);

	const auto tokenLabel = box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			settings.token(),
			st::boxLabel),
		st::settingsCheckboxPadding);

	const auto copyBtn = box->addRow(
		object_ptr<Ui::RoundButton>(
			box,
			rpl::single(u"Скопировать токен"_q),
			st::defaultBoxButton),
		st::settingsCheckboxPadding);

	copyBtn->setClickedCallback([=] {
		QGuiApplication::clipboard()->setText(GlobalSettings().token());
		Ui::Toast::Show(u"Токен скопирован в буфер обмена"_q);
	});

	const auto regenBtn = box->addRow(
		object_ptr<Ui::RoundButton>(
			box,
			rpl::single(u"Сгенерировать новый токен"_q),
			st::defaultBoxButton),
		st::settingsCheckboxPadding);

	regenBtn->setClickedCallback([=] {
		GlobalSettings().regenerateToken();
		tokenLabel->setText(GlobalSettings().token());
		Ui::Toast::Show(u"Сгенерирован новый токен"_q);
	});

	box->addButton(tr::lng_close(), [=] {
		box->closeBox();
	});
}

} // namespace Agent
