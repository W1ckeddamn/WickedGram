/*
This file is part of WickedGram Desktop,
an open-source fork of Telegram Desktop.
*/
#include "wicked/wicked_settings_box.h"

#include "wicked/wicked_config.h"
#include "wicked/wicked_vault.h"
#include "wicked/wicked_backup.h"
#include "wicked/wicked_network_boost.h"
#include "base/timer.h"
#include "lang/lang_keys.h"
#include "settings/settings_common.h"
#include "ui/boxes/confirm_box.h"
#include "ui/widgets/checkbox.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/buttons.h"
#include "ui/toast/toast.h"
#include "ui/layers/generic_box.h"
#include "styles/style_boxes.h"
#include "styles/style_layers.h"
#include "styles/style_settings.h"
#include "styles/style_menu_icons.h"

#include <QtCore/QDir>
#include <QtCore/QPointer>

namespace Wicked {

namespace {

void AddDescription(not_null<Ui::GenericBox*> box, const QString &text) {
	box->addRow(
		object_ptr<Ui::FlatLabel>(box, text, st::boxDividerLabel),
		st::settingsCheckboxPadding);
}

void AddHeading(not_null<Ui::GenericBox*> box, const QString &text) {
	box->addSkip(st::defaultSubsectionTitlePadding.top());
	box->addRow(
		object_ptr<Ui::FlatLabel>(box, text, st::defaultSubsectionTitle),
		st::defaultSubsectionTitlePadding);
}

void PrepareSection(not_null<Ui::GenericBox*> box, const QString &title) {
	box->setTitle(title);
	box->setWidth(st::boxWideWidth);
	box->addButton(rpl::single(u"Назад"_q), [=] {
		box->closeBox();
	});
}

void AddToggle(
		not_null<Ui::GenericBox*> box,
		const QString &text,
		rpl::producer<bool> value,
		Fn<void(bool)> save) {
	const auto button = box->addRow(
		object_ptr<Ui::SettingsButton>(
			box,
			rpl::single(text),
			st::settingsButtonNoIcon),
		QMargins());
	button->toggleOn(std::move(value));
	button->toggledChanges() | rpl::on_next(std::move(save), button->lifetime());
}

void PrivacyBox(not_null<Ui::GenericBox*> box) {
	PrepareSection(box, u"Приватность и защита"_q);

	AddHeading(box, u"Сообщения"_q);

	// Toggle: Bypass Content Protection
	AddToggle(
		box,
		u"Обход защиты контента"_q,
		Config::BypassContentProtectionValue(),
		Config::SetBypassContentProtection);

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Разрешает копирование текста, пересылку сообщений и сохранение любых медиафайлов из каналов и чатов с ограниченным доступом."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	// Toggle: Ghost Mode
	AddToggle(
		box,
		u"Режим невидимки"_q,
		Config::GhostModeValue(),
		Config::SetGhostMode);

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Скрывает факт прочтения входящих сообщений, голосовых заметок и просмотр историй от собеседников."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	// Toggle: Stealth Typing
	AddToggle(
		box,
		u"Скрывать статус набора"_q,
		Config::StealthTypingValue(),
		Config::SetStealthTyping);

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Не отправляет собеседникам статус «печатает...» или запись голосовых сообщений."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	// Toggle: Anti-Delete
	AddToggle(
		box,
		u"Анти-удаление сообщений"_q,
		Config::AntiDeleteMessagesValue(),
		Config::SetAntiDeleteMessages);

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Сохраняет удалённые собеседником сообщения в чате и помечает их меткой 🗑 [Удалено]."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	// Toggle: Edit History
	AddToggle(
		box,
		u"История редактирования"_q,
		Config::SaveEditHistoryValue(),
		Config::SetSaveEditHistory);

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Сохраняет исходный текст сообщений до правок собеседника и показывает старую версию."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	AddHeading(box, u"Для трансляций"_q);

	// Streamer Mode
	AddToggle(
		box,
		u"Режим стримера"_q,
		Config::StreamerModeValue(),
		Config::SetStreamerMode);

	AddToggle(
		box,
		u"Скрывать другие аккаунты"_q,
		Config::StreamerHideOtherAccountsValue(),
		Config::SetStreamerHideOtherAccounts);

	AddToggle(
		box,
		u"Скрывать номера телефонов"_q,
		Config::StreamerHidePhoneNumbersValue(),
		Config::SetStreamerHidePhoneNumbers);

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Маскирует телефонные номера и скрывает дополнительные аккаунты от случайного показа во время трансляции экрана."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);
}

void InterfaceBox(not_null<Ui::GenericBox*> box) {
	PrepareSection(box, u"Интерфейс и удобство"_q);

	AddHeading(box, u"Ссылки"_q);

	// Toggle: Social Link Fixer
	AddToggle(
		box,
		u"Автоисправление ссылок соцсетей"_q,
		Config::SocialLinkFixerValue(),
		Config::SetSocialLinkFixer);

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Автоматически заменяет ссылки Twitter/X, Instagram, TikTok, Reddit на фикс-домены для прямого предпросмотра."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	// Toggle: Clean URLs
	AddToggle(
		box,
		u"Убирать метки отслеживания"_q,
		Config::CleanUrlsValue(),
		Config::SetCleanUrls);

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Удаляет отслеживающие параметры (?utm_*, fbclid, si, igshid, ref) из отправляемых ссылок."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	AddHeading(box, u"Чаты и плеер"_q);

	// Toggle: Lyrics in Player
	AddToggle(
		box,
		u"Тексты песен в плеере"_q,
		Config::LyricsInPlayerValue(),
		Config::SetLyricsInPlayer);

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Отображает кнопку просмотра синхронизированных текстов песен и караоке из базы LRCLIB в плеере аудио и контекстном меню."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	// Toggle: Zalgo Filter
	AddToggle(
		box,
		u"Фильтровать искажения текста"_q,
		Config::FilterZalgoValue(),
		Config::SetFilterZalgo);

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Срезает избыточные комбинирующие диакритические знаки и скрытые символы инверсии направления текста."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	// Toggle: Show Seconds in Time
	AddToggle(
		box,
		u"Секунды во времени сообщений"_q,
		Config::ShowSecondsInTimeValue(),
		Config::SetShowSecondsInTime);

	AddHeading(box, u"Профили и аккаунты"_q);

	// Toggle: Custom Badges
	AddToggle(
		box,
		u"Бейджи разработчиков"_q,
		Config::CustomBadgesEnabledValue(),
		Config::SetCustomBadgesEnabled);

	// Toggle: Registration Date
	AddToggle(
		box,
		u"Дата регистрации в профиле"_q,
		Config::ShowRegistrationDateValue(),
		Config::SetShowRegistrationDate);

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Показывает оценочную дату регистрации пользователя в Telegram на основе алгоритма интерполяции ID."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	// Toggle: Real Name Cache
	AddToggle(
		box,
		u"Сохранять исходные имена"_q,
		Config::ShowOriginalNameValue(),
		Config::SetShowOriginalName);

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Сохраняет изначальные имена пользователей и отображает их, если собеседник сменил имя или был переименован."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	// Toggle: Unlimited Accounts
	AddToggle(
		box,
		u"До 100 аккаунтов"_q,
		Config::UnlimitedAccountsValue(),
		Config::SetUnlimitedAccounts);

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Позволяет подключить до 100 аккаунтов без подписки Telegram Premium."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);
}

void VaultBox(not_null<Ui::GenericBox*> box) {
	PrepareSection(box, u"Хранилище медиа"_q);

	// Toggle: Auto-Vault
	AddToggle(
		box,
		u"Автосохранение медиа"_q,
		Config::AutoVaultMediaValue(),
		Config::SetAutoVaultMedia);

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Автоматически изолирует полученные фото, видео и документы в защищённое хранилище на диске при получении."_q,
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

	const auto rotateBtn = box->addRow(
		object_ptr<Ui::SettingsButton>(
			box,
			rpl::single(u"Очистить старые файлы"_q),
			st::settingsButtonNoIcon),
		QMargins());

	rotateBtn->setClickedCallback([=] {
		const auto res = Vault::Rotate(Config::VaultTtlDays(), Config::VaultMaxSizeGb());
		vaultStatsLabel->setText(formatStats());
		const auto freedMb = double(res.freedBytes) / (1024.0 * 1024.0);
		Ui::Toast::Show(QString(u"Очистка завершена: удалено %1 файлов (%2 МБ)"_q)
			.arg(res.deletedCount)
			.arg(QString::number(freedMb, 'f', 2)));
	});

	const auto clearBtn = box->addRow(
		object_ptr<Ui::SettingsButton>(
			box,
			rpl::single(u"Удалить все файлы хранилища"_q),
			st::settingsAttentionButton),
		QMargins());

	clearBtn->setClickedCallback([=] {
		box->getDelegate()->show(Ui::MakeConfirmBox({
			.text = u"Удалить все файлы из хранилища WickedGram? Это действие нельзя отменить."_q,
			.confirmed = [=, label = QPointer<Ui::FlatLabel>(vaultStatsLabel)](Fn<void()> close) {
				close();
				if (!label) {
					return;
				}
				const auto cleared = Vault::Clear();
				label->setText(formatStats());
				Ui::Toast::Show(cleared
					? u"Хранилище медиа очищено"_q
					: u"Некоторые файлы не удалось удалить"_q);
			},
			.confirmText = tr::lng_box_delete(),
			.confirmStyle = &st::attentionBoxButton,
		}));
	});
}

void NetworkBox(not_null<Ui::GenericBox*> box) {
	PrepareSection(box, u"Сеть и загрузки"_q);

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Многопоточный параллельный акселератор MTProto. Увеличивает количество одновременных соединений и размер буфера для максимальной скорости загрузки и выгрузки файлов."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	const auto formatSpeedLabel = [] {
		return QString(u"Текущая скорость: ⬇ %1   ⬆ %2"_q)
			.arg(NetworkBoost::FormatSpeed(NetworkBoost::DownloadSpeedMBps()))
			.arg(NetworkBoost::FormatSpeed(NetworkBoost::UploadSpeedMBps()));
	};

	const auto speedLabel = box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			formatSpeedLabel(),
			st::boxLabel),
		st::settingsCheckboxPadding);

	const auto speedTimer = std::make_shared<base::Timer>([=] {
		speedLabel->setText(formatSpeedLabel());
	});
	speedTimer->callEach(1000);
	box->lifetime().add([speedTimer] {
		speedTimer->cancel();
	});

	// Download Boost Group
	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Скачивание"_q,
			st::boxLabel),
		st::settingsCheckboxPadding);

	const auto downloadGroup = std::make_shared<Ui::RadioenumGroup<NetworkBoostMode>>(
		Config::DownloadBoost());
	downloadGroup->setChangedCallback([=](NetworkBoostMode value) {
		Config::SetDownloadBoost(value);
	});

	box->addRow(
		object_ptr<Ui::Radioenum<NetworkBoostMode>>(
			box,
			downloadGroup,
			NetworkBoostMode::Off,
			u"Стандартный · 1–4 потока"_q,
			st::defaultBoxCheckbox),
		st::settingsCheckboxPadding);

	box->addRow(
		object_ptr<Ui::Radioenum<NetworkBoostMode>>(
			box,
			downloadGroup,
			NetworkBoostMode::Fast,
			u"Быстрый · до 8 потоков"_q,
			st::defaultBoxCheckbox),
		st::settingsCheckboxPadding);

	box->addRow(
		object_ptr<Ui::Radioenum<NetworkBoostMode>>(
			box,
			downloadGroup,
			NetworkBoostMode::Ultra,
			u"Ультра · 8 потоков, буфер 8 МБ"_q,
			st::defaultBoxCheckbox),
		st::settingsCheckboxPadding);

	// Upload Boost Group
	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Отправка файлов"_q,
			st::boxLabel),
		st::settingsCheckboxPadding);

	const auto uploadGroup = std::make_shared<Ui::RadioenumGroup<NetworkBoostMode>>(
		Config::UploadBoost());
	uploadGroup->setChangedCallback([=](NetworkBoostMode value) {
		Config::SetUploadBoost(value);
	});

	box->addRow(
		object_ptr<Ui::Radioenum<NetworkBoostMode>>(
			box,
			uploadGroup,
			NetworkBoostMode::Off,
			u"Стандартный · 1–4 потока"_q,
			st::defaultBoxCheckbox),
		st::settingsCheckboxPadding);

	box->addRow(
		object_ptr<Ui::Radioenum<NetworkBoostMode>>(
			box,
			uploadGroup,
			NetworkBoostMode::Fast,
			u"Быстрый · до 6 потоков"_q,
			st::defaultBoxCheckbox),
		st::settingsCheckboxPadding);

	box->addRow(
		object_ptr<Ui::Radioenum<NetworkBoostMode>>(
			box,
			uploadGroup,
			NetworkBoostMode::Ultra,
			u"Ультра · до 8 потоков"_q,
			st::defaultBoxCheckbox),
		st::settingsCheckboxPadding);

	// Toggle: Adaptive File Size Aware
	AddToggle(
		box,
		u"Учитывать размер файлов"_q,
		Config::UploadBoostFileSizeAwareValue(),
		Config::SetUploadBoostFileSizeAware);

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Автоматически выбирает стандартную скорость для небольших файлов (<10 МБ) и максимальный разгон 8 потоков для тяжёлых файлов (>100 МБ)."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);
}

void BackupBox(not_null<Ui::GenericBox*> box) {
	PrepareSection(box, u"Резервные копии"_q);

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Позволяет экспортировать все настройки WickedGram в переносимый JSON-файл и восстанавливать их на любом ПК."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	const auto exportBackupBtn = box->addRow(
		object_ptr<Ui::SettingsButton>(
			box,
			rpl::single(u"Сохранить резервную копию"_q),
			st::settingsButtonNoIcon),
		QMargins());

	exportBackupBtn->setClickedCallback([=] {
		const auto path = BackupManager::ExportBackup();
		if (!path.isEmpty()) {
			Ui::Toast::Show(u"Настройки экспортированы в tdata/backups/"_q);
		} else {
			Ui::Toast::Show(u"Ошибка экспорта настроек"_q);
		}
	});

	const auto importBackupBtn = box->addRow(
		object_ptr<Ui::SettingsButton>(
			box,
			rpl::single(u"Восстановить последнюю копию"_q),
			st::settingsButtonNoIcon),
		QMargins());

	importBackupBtn->setClickedCallback([=] {
		QDir dir(BackupManager::DefaultBackupDirectory());
		const auto list = dir.entryInfoList(QStringList() << u"*.json"_q, QDir::Files, QDir::Time);
		if (list.isEmpty()) {
			Ui::Toast::Show(u"Файлы резервных копий не найдены"_q);
			return;
		}
		if (BackupManager::ImportBackup(list.first().absoluteFilePath())) {
			Ui::Toast::Show(u"Настройки успешно восстановлены!"_q);
		} else {
			Ui::Toast::Show(u"Не удалось загрузить файл резервной копии"_q);
		}
	});
}

void AddNavigation(
		not_null<Ui::GenericBox*> box,
		const QString &title,
		const QString &description,
		const style::icon &icon,
		Fn<void(not_null<Ui::GenericBox*>)> build) {
	const auto button = box->addRow(
		Settings::CreateButtonWithIcon(
			box,
			rpl::single(title),
			st::settingsButton,
			{ .icon = &icon, .type = Settings::IconType::Simple }),
		QMargins());
	button->setClickedCallback([=] {
		box->getDelegate()->show(Box(build));
	});
	box->addRow(
		object_ptr<Ui::FlatLabel>(box, description, st::boxDividerLabel),
		{ st::settingsButton.padding.left(), 0,
			st::settingsButton.padding.right(),
			st::settingsCheckboxPadding.bottom() });
}

} // namespace

void SettingsBox(not_null<Ui::GenericBox*> box) {
	box->setTitle(u"WickedGram"_q);
	box->setWidth(st::boxWideWidth);
	AddDescription(box, u"Настройте мессенджер под себя. Все изменения сохраняются автоматически."_q);
	box->addSkip(st::defaultSubsectionTitlePadding.top());

	AddNavigation(
		box,
		u"Приватность и защита"_q,
		u"Невидимка, сообщения и режим стримера"_q,
		st::menuIconStealth,
		PrivacyBox);

	AddNavigation(
		box,
		u"Интерфейс и удобство"_q,
		u"Ссылки, плеер, профили и аккаунты"_q,
		st::menuIconPalette,
		InterfaceBox);

	AddNavigation(
		box,
		u"Хранилище медиа"_q,
		u"Автосохранение файлов и управление местом"_q,
		st::menuIconArchive,
		VaultBox);

	AddNavigation(
		box,
		u"Сеть и загрузки"_q,
		u"Режимы ускорения и текущая скорость"_q,
		st::menuIconDownload,
		NetworkBox);

	AddNavigation(
		box,
		u"Резервные копии"_q,
		u"Перенос и восстановление настроек"_q,
		st::menuIconRestore,
		BackupBox);

	box->addButton(tr::lng_close(), [=] {
		box->closeBox();
	});
}

} // namespace Wicked
