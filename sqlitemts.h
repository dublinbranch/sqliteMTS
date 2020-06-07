#pragma once

#ifndef QBL
#define QBL(str) QByteArrayLiteral(str)
#define QSL(str) QStringLiteral(str)
#endif

#include "sqlite3pp/src/sqlite3pp.h"
#include <QString>
#include <memory>
#include <mutex>
class SqliteMTS : sqlite3pp::noncopyable {
      public:
	SqliteMTS()  {};
	SqliteMTS(QByteArray db);
	int                               connect(QByteArray db);
	std::unique_ptr<sqlite3pp::query> fetch(QByteArray sql);
	bool                              execute(QByteArray sql);
	bool                              execute(sqlite3pp::command& cmd);
	sqlite3pp::database               db;

      private:
	std::mutex mutex;
};

class SqliteTool : sqlite3pp::noncopyable {
      public:
	SqliteTool(){};
	bool setDb(const QByteArray db);
	SqliteTool(const QByteArray db);
	bool runnable(const QString& key, qint64 second);

	bool hasTable(QByteArray table);

      private:
	SqliteMTS sqlite;
	bool      runnableOK = false;
	bool      prepareRunnable();
};
