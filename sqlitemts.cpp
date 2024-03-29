#include "sqlitemts.h"
#include "rbk/QStacker/qstacker.h"
#include "sqlite3.h"
#include <QDateTime>
#include <QDebug>

SqliteMTS::SqliteMTS(QByteArray _db) {
	connect(_db);
}

int SqliteMTS::connect(QByteArray _db) {
	auto status = this->db.connect(_db.constData(), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
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
	QString                     errorMsg;
	try {
		cmd.execute_all();
		auto errNum = this->db.error_code();
		switch (errNum) {
		case SQLITE_DONE:
		case SQLITE_OK:
		case SQLITE_ROW:
			return true;
		case SQLITE_READONLY:
			errorMsg = this->db.error_msg();
			errorMsg += ", not only the file must be writable but ALSO THE FOLDER!";
			break;
		default:
			errorMsg = this->db.error_msg();
			break;
		}
		qWarning().noquote() << "error in statement" << errorMsg << QStacker16Light();
		return false;
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
	prepareRunnable();
	return true;
}

SqliteTool::SqliteTool(const QByteArray db) {
	setDb(db);
}

bool SqliteTool::runnable_64(const QString& key, qint64 second, float cdMultiplier) {
	return runnable(key.toUtf8().toBase64(), second, cdMultiplier);
}

// viene usato il coolDown che è stato indicato alla chiamata precedente
bool SqliteTool::runnable(const QString& key, qint64 second, float cdMultiplier) {
	prepareRunnable();
	int  lastRun, coolDown = second;
	auto now = QDateTime::currentSecsSinceEpoch();
	{
		static const QString skel = "SELECT lastRun,coolDown FROM runnable WHERE operationCode = '%1' ORDER BY lastRun DESC LIMIT 1";
		QByteArray           sql  = skel.arg(key).toUtf8();
		auto                 res  = this->sqlite.fetch(sql);
		for (auto row : *res) {
			row.getter(0) >> lastRun;
			row.getter(1) >> coolDown;
			if (cdMultiplier > 0) {
				coolDown = coolDown * cdMultiplier;
			}
			if (lastRun + coolDown > now) {
				return false;
			}
		}
	}
	{
		sqlite3pp::command cmd(this->sqlite.db, "INSERT INTO runnable (operationCode,lastRun,coolDown) VALUES (?,?,?)");
		auto               k = key.toUtf8();

		cmd.binder() << k.constData() << now << coolDown;
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
     `lastRun` INTEGER,
     `coolDown` INTEGER
);
CREATE INDEX lastRun ON runnable(lastRun);
)";
		this->sqlite.execute(sql);
	}
	runnableOK = true;
	return runnableOK;
}
