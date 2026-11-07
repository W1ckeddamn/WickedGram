#include "wicked/vault_cleanup.h"

#include <QtCore/QTemporaryDir>
#include <iostream>

int main() {
	auto failures = 0;
	const auto check = [&](bool ok, const char *name) {
		if (!ok) {
			std::cerr << name << std::endl;
			++failures;
		}
	};
	const auto now = QDateTime::currentDateTimeUtc();
	const auto create = [&](const QString &directory, const QString &name, int size, int days) {
		auto file = QFile(directory + '/' + name);
		check(file.open(QIODevice::WriteOnly), "fixture opens");
		check(file.write(QByteArray(size, 'x')) == size, "fixture writes");
		check(file.flush(), "fixture flushes");
		check(file.setFileTime(now.addDays(-days), QFileDevice::FileModificationTime),
			"fixture timestamp");
	};
	{
		auto directory = QTemporaryDir();
		check(directory.isValid(), "TTL directory");
		if (!directory.isValid()) {
			return 1;
		}
		const auto path = directory.path();
		create(path, "1_2_3_old.bin", 10, 10);
		create(path, "1_2_4_new.bin", 20, 1);
		create(path, "1_9_3_protected.bin", 30, 10);
		create(path, "9_3_legacy.bin", 40, 10);
		create(path, "2_9_3_other-account.bin", 15, 10);
		create(path, "notes.txt", 50, 10);
		const auto result = Wicked::VaultCleanup::Run(path, 7, 0, { "1:9" }, now);
		check(result.deletedCount == 2 && result.freedBytes == 25, "TTL accounting");
		check(!QFile::exists(path + "/1_2_3_old.bin"), "expired file removed");
		check(QFile::exists(path + "/1_2_4_new.bin"), "recent file retained");
		check(QFile::exists(path + "/1_9_3_protected.bin"), "protected file retained");
		check(QFile::exists(path + "/9_3_legacy.bin"), "legacy protected file retained");
		check(QFile::exists(path + "/notes.txt"), "unmanaged file retained");
		check(!QFile::exists(path + "/2_9_3_other-account.bin"), "account-specific protection");
	}
	{
		auto directory = QTemporaryDir();
		if (!directory.isValid()) {
			return 1;
		}
		const auto path = directory.path();
		create(path, "1_2_1_old.bin", 10, 10);
		create(path, "1_2_2_new.bin", 20, 1);
		create(path, "1_9_3_protected.bin", 30, 20);
		const auto result = Wicked::VaultCleanup::Run(path, 0, 50, { "1:9" }, now);
		check(result.deletedCount == 1 && result.freedBytes == 10, "quota accounting");
		check(QFile::exists(path + "/1_2_2_new.bin"), "quota removes oldest first");
		const auto excess = Wicked::VaultCleanup::Run(path, 0, 1, { "1:9" }, now);
		check(excess.deletedCount == 1 && excess.freedBytes == 20, "protected quota excess");
		check(QFile::exists(path + "/1_9_3_protected.bin"), "protection exceeds quota");
		const auto unlimited = Wicked::VaultCleanup::Run(path, 0, 0, {}, now);
		check(unlimited.deletedCount == 0 && unlimited.freedBytes == 0, "unlimited is inert");
	}
	std::cout << "Vault cleanup failures: " << failures << std::endl;
	return failures ? 1 : 0;
}
