#include "sqlitemts.h"
#include "sqlite3.h"
#include <QDateTime>

SqliteMTS::SqliteMTS() {
}

SqliteMTS::SqliteMTS(QByteArray db) {
  connect(db);
}

int SqliteMTS::connect(QByteArray db) {
  return this->db.connect(db.constData(), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
}

std::unique_ptr<sqlite3pp::query> SqliteMTS::query(QByteArray sql) {
  std::lock_guard<std::mutex>       lock_guard(mutex);
  std::unique_ptr<sqlite3pp::query> res = std::make_unique<sqlite3pp::query>(this->db, sql);
  return res;
}

/**
 * @brief coolDown will check the last time a "key" has been activated
 * @param key
 * @param time
 * @return true: we can run, false: do not run
 */
SqliteTool::SqliteTool(QByteArray db) {
  this->sqlite.connect(db);
}

bool SqliteTool::runnable(const QString& key, qint64 second) {
  static const QString skel = "SELECT lastRun FROM runnable WHERE operationCode = %1 ORDER BY lastRun DESC LIMIT 1";
  auto                 now  = QDateTime::currentSecsSinceEpoch();
  QByteArray           sql  = skel.arg(key).toUtf8();
  auto                 res  = this->sqlite.query(sql);
  for (auto v : *res) {
    int lastRun;
    v.getter() >> lastRun;
    if (lastRun + second > now) {
      return false;
    }
  }
  return true;
}

bool SqliteTool::hasTable(QByteArray table)
{
    QByteArray sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='" + table + "';";
    auto res = this->sqlite.query(sql);
}

bool SqliteTool::getRunnableOK() {
  //this will just check  if the table exist, if not create it (used for new database)
  if (runnableOK) {
    return runnableOK;
  }


  return runnableOK;
}
