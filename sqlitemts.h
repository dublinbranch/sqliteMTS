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
	SqliteMTS(){};
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
	SqliteTool(const QByteArray db);
	/**
	 * @brief runnable_64
	 * @param key
	 * @param second of cooldown
	 * @param cdMultiplier increases each time the cooldown
	 * @return 
	 */
	bool runnable_64(const QString& key, qint64 second, float cdMultiplier = 0);
	/**
	 * @brief runnable
	 * @param key
	 * @param second of cooldown
	 * @param cdMultiplier increases each time the cooldown
	 * @return 
	 */
	bool runnable(const QString& key, qint64 second, float cdMultiplier = 0);

	bool hasTable(QByteArray table);

      private:
	SqliteMTS sqlite;
	bool      setDb(const QByteArray db);
	bool      runnableOK = false;
	//Non thread safe init called before usage when setting the DB
	bool prepareRunnable();
};
