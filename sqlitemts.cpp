#include "sqlitemts.h"
#include "QStacker/qstacker.h"
#include "sqlite3.h"
#include <QDateTime>
#include <QDebug>

SqliteMTS::SqliteMTS(QByteArray db) {
	connect(db);
}

int SqliteMTS::connect(QByteArray db) {
	auto status = this->db.connect(db.constData(), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
	this->db.enable_extended_result_codes();
	return status;
}

std::unique_ptr<sqlite3pp::query> SqliteMTS::fetch(QByteArray sql) {
	std::lock_guard<std::mutex> lock_guard(mutex);
	auto                        res = std::make_unique<sqlite3pp::query>(this->db);
	try {
		res->prepare(sql);
		return res;
	} catch (...) {
	}
	return res;
}

bool SqliteMTS::execute(QByteArray sql) {
	std::lock_guard<std::mutex> lock_guard(mutex);
	try {
		this->db.execute(sql);

		auto errNum = this->db.error_code();
		if (errNum != 0) {
			auto errMsg = this->db.error_msg();
			qWarning().noquote() << "error in " << sql << errMsg << QStacker16Light();
			return false;
		}
	} catch (...) {
		return false;
	}
	return true;
}

bool SqliteMTS::execute(sqlite3pp::command& cmd) {
	std::lock_guard<std::mutex> lock_guard(mutex);
	try {
		cmd.execute_all();
		auto errNum = this->db.error_code();
		if (errNum != 0 && errNum != 101) {
			auto errMsg = this->db.error_msg();
			qWarning().noquote() << "error in statement" << errMsg << QStacker16Light();
			return false;
		}
	} catch (...) {
		return false;
	}
	return true;
}

/**
 * @brief coolDown will check the last time a "key" has been activated
 * @param key
 * @param time
 * @return true: we can run, false: do not run
 */
bool SqliteTool::setDb(const QByteArray db) {
	this->sqlite.connect(db);
	//TODO fai dei check
	return true;
}

SqliteTool::SqliteTool(const QByteArray db) {
	setDb(db);
}

bool SqliteTool::runnable_64(const QString& key, qint64 second) {
	return runnable(key.toUtf8().toBase64(), second);
}

bool SqliteTool::runnable(const QString& key, qint64 second) {
	prepareRunnable();
	auto now = QDateTime::currentSecsSinceEpoch();
	{
		static const QString skel = "SELECT lastRun FROM runnable WHERE operationCode = '%1' ORDER BY lastRun DESC LIMIT 1";

		QByteArray sql = skel.arg(key).toUtf8();
		auto       res = this->sqlite.fetch(sql);
		for (auto row : *res) {
			int lastRun;
			row.getter() >> lastRun;
			if (lastRun + second > now) {
				return false;
			}
		}
	}
	{
		sqlite3pp::command cmd(this->sqlite.db, "INSERT INTO runnable (operationCode,lastRun) VALUES (?,?)");
		auto               k = key.toUtf8();
		cmd.binder() << k.constData() << now;
		this->sqlite.execute(cmd);
	}
	return true;
}

bool SqliteTool::hasTable(QByteArray table) {
	QByteArray sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='" + table + "';";
	auto       res = this->sqlite.fetch(sql);
	if (!res->column_count()) {
		return false;
	}
	for (auto row : *res) {
		std::string name;
		row.getter() >> name;
		if (name == table.toStdString()) {
			return true;
		}
	}
	return false;
}

bool SqliteTool::prepareRunnable() {
	//this will just check  if the table exist, if not create it (used for new database)
	if (runnableOK) {
		return runnableOK;
	}
	if (!hasTable("runnable")) {
		auto sql = R"(
CREATE TABLE `runnable` (
     `id` INTEGER PRIMARY KEY AUTOINCREMENT,
     `operationCode` TEXT,
     `lastRun` INTEGER
);
CREATE INDEX lastRun ON runnable(lastRun);
)";
		this->sqlite.execute(sql);
	}
	runnableOK = true;
	return runnableOK;
}
