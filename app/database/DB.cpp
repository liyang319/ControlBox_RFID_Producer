// DB.cpp

#include "DB.h"
#include <fstream>
#include <iostream>
#include <sstream>

DB::DB(const std::string &dbPath) : m_db(nullptr), m_dbPath(dbPath) {}

DB::~DB()
{
    if (m_db)
    {
        close();
    }
}

// bool DB::lockDB()
// {
//     m_mutex.lock();
//     return true;
// }

// bool DB::unlockDB()
// {
//     m_mutex.unlock();
//     return true;
// }

bool DB::open()
{
    int result = sqlite3_open(m_dbPath.c_str(), &m_db);
    if (result == SQLITE_OK)
    {
        return true;
    }
    else if (result == SQLITE_CANTOPEN)
    {
        // 创建数据库文件
        std::ofstream file(m_dbPath);
        if (!file.is_open())
        {
            return false;
        }
        file.close();

        // 再次尝试打开数据库
        if (sqlite3_open(m_dbPath.c_str(), &m_db) == SQLITE_OK)
        {
            return true;
        }
    }
    return false;
}

bool DB::close()
{
    if (sqlite3_close(m_db) != SQLITE_OK)
    {
        return false;
    }
    m_db = nullptr;
    return true;
}

bool DB::execute(const std::string &sql)
{
    char *errMsg = nullptr;
    if (sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool DB::createTable(const std::string &tableName, const std::string &columns)
{
    if (tableExists(tableName))
    {
        std::cout << "Table " << tableName << " already exists." << std::endl;
        return true;
    }
    std::string sql = "CREATE TABLE " + tableName + " (" + columns + ");";
    return execute(sql);
}

bool DB::insertData(const std::string &tableName, const std::string &values)
{
    std::string sql = "INSERT INTO " + tableName + " VALUES (" + values + ");";
    std::cout << sql << std::endl;
    return execute(sql);
}

bool DB::updateData(const std::string &tableName, const std::string &setValues, const std::string &condition)
{
    std::string sql = "UPDATE " + tableName + " SET " + setValues + " WHERE " + condition + ";";
    return execute(sql);
}

bool DB::deleteData(const std::string &tableName, const std::string &condition)
{
    std::string sql = "DELETE FROM " + tableName + " WHERE " + condition + ";";
    return execute(sql);
}

bool DB::tableExists(const std::string &tableName)
{
    std::string sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='" + tableName + "';";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK)
    {
        return false;
    }
    bool exists = false;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        exists = true;
    }
    sqlite3_finalize(stmt);
    return exists;
}

bool DB::isDbEmpty(const std::string &tableName)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    std::string sql = "SELECT COUNT(*) FROM " + tableName + ";";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK)
    {
        std::cout << "Failed to prepare statement for counting data." << std::endl;
        return true; // 默认数据库为空
    }

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int count = sqlite3_column_int(stmt, 0);

        return count == 0;
    }

    sqlite3_finalize(stmt);

    return true; // 默认数据库为空
}
