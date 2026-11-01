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
#include "ui/widgets/checkbox.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/buttons.h"
#include "ui/toast/toast.h"
#include "ui/layers/generic_box.h"
#include "styles/style_boxes.h"
#include "styles/style_layers.h"
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
	) | rpl::on_next([=](bool checked) {
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
	) | rpl::on_next([=](bool checked) {
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
	) | rpl::on_next([=](bool checked) {
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
	) | rpl::on_next([=](bool checked) {
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
	) | rpl::on_next([=](bool checked) {
		Config::SetSaveEditHistory(checked);
	}, box->lifetime());

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Сохраняет исходный текст сообщений до правок собеседника и показывает старую версию."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	// Streamer Mode
	const auto streamerCheckbox = box->addRow(
		object_ptr<Ui::Checkbox>(
			box,
			u"Режим стримера (Streamer Mode)"_q,
			Config::StreamerMode()),
		st::settingsCheckboxPadding);

	streamerCheckbox->checkedChanges(
	) | rpl::on_next([=](bool checked) {
		Config::SetStreamerMode(checked);
	}, box->lifetime());

	const auto hideAccountsCheckbox = box->addRow(
		object_ptr<Ui::Checkbox>(
			box,
			u"Скрывать другие аккаунты в меню стримера"_q,
			Config::StreamerHideOtherAccounts()),
		st::settingsCheckboxPadding);

	hideAccountsCheckbox->checkedChanges(
	) | rpl::on_next([=](bool checked) {
		Config::SetStreamerHideOtherAccounts(checked);
	}, box->lifetime());

	const auto hidePhonesCheckbox = box->addRow(
		object_ptr<Ui::Checkbox>(
			box,
			u"Скрывать телефонные номера в интерфейсе (+• ••• ••• •• ••)"_q,
			Config::StreamerHidePhoneNumbers()),
		st::settingsCheckboxPadding);

	hidePhonesCheckbox->checkedChanges(
	) | rpl::on_next([=](bool checked) {
		Config::SetStreamerHidePhoneNumbers(checked);
	}, box->lifetime());

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Маскирует телефонные номера и скрывает дополнительные аккаунты от случайного показа во время трансляции экрана."_q,
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
	) | rpl::on_next([=](bool checked) {
		Config::SetSocialLinkFixer(checked);
	}, box->lifetime());

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Автоматически заменяет ссылки Twitter/X, Instagram, TikTok, Reddit на фикс-домены для прямого предпросмотра."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	// Toggle: Clean URLs
	const auto cleanUrlsCheckbox = box->addRow(
		object_ptr<Ui::Checkbox>(
			box,
			u"Очистка ссылок от трекеров и utm-меток (Clean URLs)"_q,
			Config::CleanUrls()),
		st::settingsCheckboxPadding);

	cleanUrlsCheckbox->checkedChanges(
	) | rpl::on_next([=](bool checked) {
		Config::SetCleanUrls(checked);
	}, box->lifetime());

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Удаляет отслеживающие параметры (?utm_*, fbclid, si, igshid, ref) из отправляемых ссылок."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	// Toggle: Lyrics in Player
	const auto lyricsCheckbox = box->addRow(
		object_ptr<Ui::Checkbox>(
			box,
			u"Тексты песен в плеере (LRCLIB Lyrics)"_q,
			Config::LyricsInPlayer()),
		st::settingsCheckboxPadding);

	lyricsCheckbox->checkedChanges(
	) | rpl::on_next([=](bool checked) {
		Config::SetLyricsInPlayer(checked);
	}, box->lifetime());

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Отображает кнопку просмотра синхронизированных текстов песен и караоке из базы LRCLIB в плеере аудио и контекстном меню."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	// Toggle: Zalgo Filter
	const auto zalgoCheckbox = box->addRow(
		object_ptr<Ui::Checkbox>(
			box,
			u"Фильтрация Zalgo-глитчей и RTL-троянов"_q,
			Config::FilterZalgo()),
		st::settingsCheckboxPadding);

	zalgoCheckbox->checkedChanges(
	) | rpl::on_next([=](bool checked) {
		Config::SetFilterZalgo(checked);
	}, box->lifetime());

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Срезает избыточные комбинирующие диакритические знаки и скрытые символы инверсии направления текста."_q,
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
	) | rpl::on_next([=](bool checked) {
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
	) | rpl::on_next([=](bool checked) {
		Config::SetCustomBadgesEnabled(checked);
	}, box->lifetime());

	// Toggle: Registration Date
	const auto regDateCheckbox = box->addRow(
		object_ptr<Ui::Checkbox>(
			box,
			u"Примерная дата регистрации в профиле"_q,
			Config::ShowRegistrationDate()),
		st::settingsCheckboxPadding);

	regDateCheckbox->checkedChanges(
	) | rpl::on_next([=](bool checked) {
		Config::SetShowRegistrationDate(checked);
	}, box->lifetime());

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Показывает оценочную дату регистрации пользователя в Telegram на основе алгоритма интерполяции ID."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	// Toggle: Real Name Cache
	const auto realNameCheckbox = box->addRow(
		object_ptr<Ui::Checkbox>(
			box,
			u"Кэшировать исходные имена пользователей"_q,
			Config::ShowOriginalName()),
		st::settingsCheckboxPadding);

	realNameCheckbox->checkedChanges(
	) | rpl::on_next([=](bool checked) {
		Config::SetShowOriginalName(checked);
	}, box->lifetime());

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Сохраняет изначальные имена пользователей и отображает их, если собеседник сменил имя или был переименован."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	// Toggle: Unlimited Accounts
	const auto unlimitedAccountsCheckbox = box->addRow(
		object_ptr<Ui::Checkbox>(
			box,
			u"Снять ограничение на количество аккаунтов (до 100)"_q,
			Config::UnlimitedAccounts()),
		st::settingsCheckboxPadding);

	unlimitedAccountsCheckbox->checkedChanges(
	) | rpl::on_next([=](bool checked) {
		Config::SetUnlimitedAccounts(checked);
	}, box->lifetime());

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Позволяет подключать неограниченное число аккаунтов без требования подписки Telegram Premium."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	// Category: Media Vault & Archiver
	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Хранилище медиа и архиватор (Vault)"_q,
			st::boxLabel),
		st::settingsCheckboxPadding);

	// Toggle: Auto-Vault
	const auto autoVaultCheckbox = box->addRow(
		object_ptr<Ui::Checkbox>(
			box,
			u"Авто-сохранение всех входящих медиа (Auto-Vault)"_q,
			Config::AutoVaultMedia()),
		st::settingsCheckboxPadding);

	autoVaultCheckbox->checkedChanges(
	) | rpl::on_next([=](bool checked) {
		Config::SetAutoVaultMedia(checked);
	}, box->lifetime());

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
		object_ptr<Ui::RoundButton>(
			box,
			rpl::single(u"Выполнить очистку по сроку и квоте"_q),
			st::defaultBoxButton),
		st::settingsCheckboxPadding);

	rotateBtn->setClickedCallback([=] {
		const auto res = Vault::Rotate(Config::VaultTtlDays(), Config::VaultMaxSizeGb());
		vaultStatsLabel->setText(formatStats());
		const auto freedMb = double(res.freedBytes) / (1024.0 * 1024.0);
		Ui::Toast::Show(QString(u"Очистка завершена: удалено %1 файлов (%2 МБ)"_q)
			.arg(res.deletedCount)
			.arg(QString::number(freedMb, 'f', 2)));
	});

	const auto clearBtn = box->addRow(
		object_ptr<Ui::RoundButton>(
			box,
			rpl::single(u"Очистить хранилище медиа полностью"_q),
			st::defaultBoxButton),
		st::settingsCheckboxPadding);

	clearBtn->setClickedCallback([=] {
		Vault::Clear();
		vaultStatsLabel->setText(formatStats());
		Ui::Toast::Show(u"Хранилище медиа очищено"_q);
	});

	// Category: Network Boost & Acceleration
	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Сеть и ускорение (Network Boost)"_q,
			st::boxLabel),
		st::settingsCheckboxPadding);

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
			u"Ускорение скачивания (Download Boost):"_q,
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
			u"Отключено (Стандарт Telegram, 1-4 потока)"_q,
			st::defaultBoxCheckbox),
		st::settingsCheckboxPadding);

	box->addRow(
		object_ptr<Ui::Radioenum<NetworkBoostMode>>(
			box,
			downloadGroup,
			NetworkBoostMode::Fast,
			u"Быстрый 2x (Ускоренный старт, до 6-8 потоков)"_q,
			st::defaultBoxCheckbox),
		st::settingsCheckboxPadding);

	box->addRow(
		object_ptr<Ui::Radioenum<NetworkBoostMode>>(
			box,
			downloadGroup,
			NetworkBoostMode::Ultra,
			u"Ультра 4x (Мгновенные 8 потоков, буфер до 8 МБ)"_q,
			st::defaultBoxCheckbox),
		st::settingsCheckboxPadding);

	// Upload Boost Group
	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Ускорение выгрузки (Upload Boost):"_q,
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
			u"Отключено (Стандарт Telegram, 1-4 потока)"_q,
			st::defaultBoxCheckbox),
		st::settingsCheckboxPadding);

	box->addRow(
		object_ptr<Ui::Radioenum<NetworkBoostMode>>(
			box,
			uploadGroup,
			NetworkBoostMode::Fast,
			u"Быстрый 2x (До 6 параллельных потоков выгрузки)"_q,
			st::defaultBoxCheckbox),
		st::settingsCheckboxPadding);

	box->addRow(
		object_ptr<Ui::Radioenum<NetworkBoostMode>>(
			box,
			uploadGroup,
			NetworkBoostMode::Ultra,
			u"Ультра 4x (До 8 параллельных потоков выгрузки)"_q,
			st::defaultBoxCheckbox),
		st::settingsCheckboxPadding);

	// Toggle: Adaptive File Size Aware
	const auto adaptiveCheckbox = box->addRow(
		object_ptr<Ui::Checkbox>(
			box,
			u"Адаптивный режим по размеру файлов (File Size Aware)"_q,
			Config::UploadBoostFileSizeAware()),
		st::settingsCheckboxPadding);

	adaptiveCheckbox->checkedChanges(
	) | rpl::on_next([=](bool checked) {
		Config::SetUploadBoostFileSizeAware(checked);
	}, box->lifetime());

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Автоматически выбирает стандартную скорость для небольших файлов (<10 МБ) и максимальный разгон 8 потоков для тяжёлых файлов (>100 МБ)."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	// Category: Backup & Restore
	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Резервное копирование (Backup & Restore)"_q,
			st::boxLabel),
		st::settingsCheckboxPadding);

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			u"Позволяет экспортировать все настройки WickedGram в переносимый JSON-файл и восстанавливать их на любом ПК."_q,
			st::boxDividerLabel),
		st::settingsCheckboxPadding);

	const auto exportBackupBtn = box->addRow(
		object_ptr<Ui::RoundButton>(
			box,
			rpl::single(u"Экспортировать настройки в файл"_q),
			st::defaultBoxButton),
		st::settingsCheckboxPadding);

	exportBackupBtn->setClickedCallback([=] {
		const auto path = BackupManager::ExportBackup();
		if (!path.isEmpty()) {
			Ui::Toast::Show(u"Настройки экспортированы в tdata/backups/"_q);
		} else {
			Ui::Toast::Show(u"Ошибка экспорта настроек"_q);
		}
	});

	const auto importBackupBtn = box->addRow(
		object_ptr<Ui::RoundButton>(
			box,
			rpl::single(u"Импортировать настройки из последнего бэкапа"_q),
			st::defaultBoxButton),
		st::settingsCheckboxPadding);

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

	box->addButton(tr::lng_close(), [=] {
		box->closeBox();
	});
}

} // namespace Wicked
