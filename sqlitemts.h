#ifndef SQLITEMTS_H
#define SQLITEMTS_H

#include "sqlite3pp/src/sqlite3pp.h"
#include <QString>
#include <memory>
#include <mutex>
class SqliteMTS : sqlite3pp::noncopyable {
public:
  SqliteMTS();
  SqliteMTS(QByteArray db);
  int                               connect(QByteArray db);
  std::unique_ptr<sqlite3pp::query> query(QByteArray sql);

private:
  std::mutex          mutex;
  sqlite3pp::database db;
};

class SqliteTool : sqlite3pp::noncopyable {
public:
  SqliteTool(QByteArray db);
  bool runnable(const QString& key, qint64 second);

  bool getRunnableOK();
  bool hasTable(QByteArray table) const;

private:
  SqliteMTS sqlite;
  bool      runnableOK = false;
};

#endif // SQLITEMTS_H
