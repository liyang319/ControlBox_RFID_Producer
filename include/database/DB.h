// DB.h

#ifndef DB_H
#define DB_H

#include <sqlite3.h>
#include <string>
#include <mutex>

class DB
{
public:
    DB(const std::string &dbPath);
    ~DB();

    bool lockDB();
    bool unlockDB();

    bool open();
    bool close();
    bool execute(const std::string &sql);

    bool createTable(const std::string &tableName, const std::string &columns);
    bool insertData(const std::string &tableName, const std::string &values);
    bool updateData(const std::string &tableName, const std::string &setValues, const std::string &condition);
    bool deleteData(const std::string &tableName, const std::string &condition);
    bool tableExists(const std::string &tableName);
    bool isDbEmpty(const std::string &tableName);
    sqlite3 *getDB() { return m_db; };
    std::mutex db_mutex;

private:
    sqlite3 *m_db;
    std::string m_dbPath;
};

#endif
