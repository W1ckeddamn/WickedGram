/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#include "wicked/wicked_settings_box.h"

#include "wicked/wicked_config.h"
#include "wicked/wicked_vault.h"
#include "lang/lang_keys.h"
#include "ui/widgets/checkbox.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/buttons.h"
#include "ui/toast/toast.h"
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

	// Toggle: Ghost Mode
	const auto ghostCheckbox = box->addRow(
		object_ptr<Ui::Checkbox>(
			box,
			u"Режим невидимки (Ghost Mode)"_q,
			Config::GhostMode()),
		st::settingsCheckboxPadding);

	ghostCheckbox->checkedChanges(
	) | rpl::start_with_next([=](bool checked) {
		Config::SetGhostMode(checked);
	}, box->lifetime());

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Скрывает факт прочтения входящих сообщений, голосовых заметок и просмотр историй от собеседников."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	// Toggle: Stealth Typing
	const auto typingCheckbox = box->addRow(
		object_ptr<Ui::Checkbox>(
			box,
			u"Скрывать статус набора (Stealth Typing)"_q,
			Config::StealthTyping()),
		st::settingsCheckboxPadding);

	typingCheckbox->checkedChanges(
	) | rpl::start_with_next([=](bool checked) {
		Config::SetStealthTyping(checked);
	}, box->lifetime());

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Не отправляет собеседникам статус «печатает...» или запись голосовых сообщений."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	// Toggle: Anti-Delete
	const auto antiDeleteCheckbox = box->addRow(
		object_ptr<Ui::Checkbox>(
			box,
			u"Анти-удаление сообщений (Anti-Delete)"_q,
			Config::AntiDeleteMessages()),
		st::settingsCheckboxPadding);

	antiDeleteCheckbox->checkedChanges(
	) | rpl::start_with_next([=](bool checked) {
		Config::SetAntiDeleteMessages(checked);
	}, box->lifetime());

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Сохраняет удалённые собеседником сообщения в чате и помечает их меткой 🗑 [Удалено]."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	// Toggle: Edit History
	const auto editHistoryCheckbox = box->addRow(
		object_ptr<Ui::Checkbox>(
			box,
			u"История редактирования (Save Edit History)"_q,
			Config::SaveEditHistory()),
		st::settingsCheckboxPadding);

	editHistoryCheckbox->checkedChanges(
	) | rpl::start_with_next([=](bool checked) {
		Config::SetSaveEditHistory(checked);
	}, box->lifetime());

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Сохраняет исходный текст сообщений до правок собеседника и показывает старую версию."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	// Category: Interface & Enhancements
	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Интерфейс и улучшения"_q,
			st::boxLabel),
		st::settingsCheckboxPadding);

	// Toggle: Social Link Fixer
	const auto socialCheckbox = box->addRow(
		object_ptr<Ui::Checkbox>(
			box,
			u"Автоисправление ссылок соцсетей (Social Link Fixer)"_q,
			Config::SocialLinkFixer()),
		st::settingsCheckboxPadding);

	socialCheckbox->checkedChanges(
	) | rpl::start_with_next([=](bool checked) {
		Config::SetSocialLinkFixer(checked);
	}, box->lifetime());

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Автоматически заменяет ссылки Twitter/X, Instagram, TikTok, Reddit на фикс-домены для прямого предпросмотра."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	// Toggle: Show Seconds in Time
	const auto secondsCheckbox = box->addRow(
		object_ptr<Ui::Checkbox>(
			box,
			u"Секунды во времени сообщений"_q,
			Config::ShowSecondsInTime()),
		st::settingsCheckboxPadding);

	secondsCheckbox->checkedChanges(
	) | rpl::start_with_next([=](bool checked) {
		Config::SetShowSecondsInTime(checked);
	}, box->lifetime());

	// Toggle: Custom Badges
	const auto badgesCheckbox = box->addRow(
		object_ptr<Ui::Checkbox>(
			box,
			u"Особые бейджи разработчиков WickedGram"_q,
			Config::CustomBadgesEnabled()),
		st::settingsCheckboxPadding);

	badgesCheckbox->checkedChanges(
	) | rpl::start_with_next([=](bool checked) {
		Config::SetCustomBadgesEnabled(checked);
	}, box->lifetime());

	// Category: Media Vault & Archiver
	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Хранилище медиа и архиватор (Vault)"_q,
			st::boxLabel),
		st::settingsCheckboxPadding);

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Медиафайлы удалённых собеседником сообщений изолируются в защищённое хранилище с помощью прямых ссылок NTFS без дублирования места на диске."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	const auto formatStats = [] {
		const auto stats = Vault::GetStats();
		const auto mb = double(stats.bytes) / (1024.0 * 1024.0);
		return QString(u"Изолированных файлов: %1 (%2 МБ)"_q)
			.arg(stats.count)
			.arg(QString::number(mb, 'f', 2));
	};

	const auto vaultStatsLabel = box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			formatStats(),
			st::boxLabel),
		st::settingsCheckboxPadding);

	const auto clearBtn = box->addRow(
		object_ptr<Ui::RoundButton>(
			box,
			rpl::single(u"Очистить хранилище медиа"_q),
			st::defaultBoxButton),
		st::settingsCheckboxPadding);

	clearBtn->setClickedCallback([=] {
		Vault::Clear();
		vaultStatsLabel->setText(formatStats());
		Ui::Toast::Show(u"Хранилище медиа очищено"_q);
	});

	box->addButton(tr::lng_close(), [=] {
		box->closeBox();
	});
}

} // namespace Wicked
