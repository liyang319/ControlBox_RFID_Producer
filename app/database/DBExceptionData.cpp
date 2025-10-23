// DBExceptionData.cpp

#include "DBExceptionData.h"
#include <string>
#include <iostream>
#include <unistd.h>
#include "DataDef.h"
#include "Utility.h"
#include "GlobalFlag.h"

DBExceptionData::DBExceptionData(const std::string &dbPath) : DB(dbPath) {}

DBExceptionData &DBExceptionData::getInstance()
{
    static DBExceptionData instance(DEFAULT_DB_EXCEPTION_PATH);
    return instance;
}

bool DBExceptionData::init()
{
    if (!open())
    {
        std::cout << "Failed to open database." << std::endl;
        return false;
    }
    // std::string columns = "timestamp INTEGER PRIMARY KEY, handleflag INTEGER, weigh_info TEXT";
    std::string columns = "timestamp INTEGER PRIMARY KEY, exceptionCode INTEGER, expSensors INTEGER, expGC31s INTEGER";
    if (createTable(DEFAULT_DB_EXCEPTION_TABLE_NAME, columns))
    {
        std::cout << "Table weigh_data created successfully." << std::endl;
    }
    else
    {
        std::cout << "Failed to create table weigh_data." << std::endl;
    }
    return true;
}

bool DBExceptionData::inserExceptionData()
{
    std::lock_guard<std::mutex> lock(db_mutex);
    if (checkExceptionData())
    {
        // std::cout << "-----event-----------insert-----" << std::endl;
        std::string values = std::to_string(Utility::getTimestamp()) + ", " + std::to_string(currentException.exceptionCode) + ", " + std::to_string(currentException.expSensors) + ", " + std::to_string(currentException.expGC31s);
        insertData(DEFAULT_DB_EXCEPTION_TABLE_NAME, values);
        return true;
    }
    return false;
}

bool DBExceptionData::checkExceptionData()
{
    if (currentException.exceptionCode != GlobalFlag::getInstance().exceptionCode || currentException.expGC31s != GlobalFlag::getInstance().expGC31s || currentException.expSensors != GlobalFlag::getInstance().expSensors)
    {
        currentException.exceptionCode = GlobalFlag::getInstance().exceptionCode;
        currentException.expGC31s = GlobalFlag::getInstance().expGC31s;
        currentException.expSensors = GlobalFlag::getInstance().expSensors;
        if (currentException.exceptionCode != 0)
        {
            return true;
        }
    }
    return false;
}

DBExceptionData::~DBExceptionData()
{
    close();
}

bool DBExceptionData::loadExceptionsFromDB()
{
    sqlite3 *db = getDB();
    if (!db)
    {
        std::cerr << "Database is not open." << std::endl;
        return false;
    }

    exceptionVec.clear();
    pageNum = 0;
    currentExpPageIndex = 0;
    // const char *sql = "SELECT timestamp, exceptionCode, expSensors, expGC31s FROM exception_table";
    std::string sql = "SELECT timestamp, exceptionCode, expSensors, expGC31s FROM " + std::string(DEFAULT_DB_EXCEPTION_TABLE_NAME) + " ORDER BY timestamp DESC";
    sqlite3_stmt *stmt;

    // 准备 SQL 语句
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // 执行查询并填充 exceptionVec
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        ExceptionDBDataUnit dataUnit;
        dataUnit.timestamp = sqlite3_column_int(stmt, 0);
        dataUnit.exceptionCode = static_cast<uint8_t>(sqlite3_column_int(stmt, 1));
        dataUnit.expSensors = static_cast<uint8_t>(sqlite3_column_int(stmt, 2));
        dataUnit.expGC31s = static_cast<uint8_t>(sqlite3_column_int(stmt, 3));

        exceptionVec.push_back(dataUnit);
    }

    // 计算 pageNum
    if (exceptionVec.size() > 0)
    {
        pageNum = (exceptionVec.size() + DEFAULT_EXCEPTIONS_UI_PAGE_SIZE - 1) / DEFAULT_EXCEPTIONS_UI_PAGE_SIZE; // 向上取整
    }
    else
    {
        pageNum = 0; // 如果没有数据，pageNum 为 0
    }

    // 清理
    sqlite3_finalize(stmt);
    return true;
}

bool DBExceptionData::getExceptions(int pageIndex, vector<ExceptionDBDataUnit> &vecExp, int pageSize)
{
    // 计算起始索引
    int startIndex = pageIndex * pageSize;
    // 计算结束索引
    int endIndex = startIndex + pageSize;
    // 获取 exceptionVec 的总大小
    int totalSize = exceptionVec.size();
    // 确保起始索引不超出范围
    if (startIndex >= totalSize)
    {
        vecExp.clear(); // 如果起始索引超出范围，清空结果并返回
        return false;
    }

    // 确保结束索引不超出范围
    if (endIndex > totalSize)
    {
        endIndex = totalSize; // 如果结束索引超出范围，调整结束索引
    }
    // 将指定范围的元素复制到 vecExp
    vecExp.clear();
    for (int i = startIndex; i < endIndex; ++i)
    {
        vecExp.push_back(exceptionVec[i]);
    }
    return !vecExp.empty(); // 返回是否成功获取到数据
}

bool DBExceptionData::deleteExpireExceptionData(int hours)
{
    std::lock_guard<std::mutex> lock(db_mutex); // 确保线程安全

    // 获取当前时间戳
    int64_t currentTimestamp = Utility::getTimestamp();
    // 计算过期时间戳
    int64_t expireTimestamp = currentTimestamp - (hours * 3600); // hours 转换为秒

    sqlite3 *db = getDB();
    if (!db)
    {
        std::cerr << "Database is not open." << std::endl;
        return false;
    }

    // 构建 SQL 查询语句
    std::string selectSql = "SELECT timestamp, exceptionCode, expSensors, expGC31s FROM " +
                            std::string(DEFAULT_DB_EXCEPTION_TABLE_NAME) +
                            " WHERE timestamp < " + std::to_string(expireTimestamp);

    sqlite3_stmt *selectStmt;

    // 准备 SQL 查询语句
    if (sqlite3_prepare_v2(db, selectSql.c_str(), -1, &selectStmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Failed to prepare select statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    std::cout << "Expired exception data (older than " << hours << " hours):" << std::endl;

    // 执行查询并打印结果
    while (sqlite3_step(selectStmt) == SQLITE_ROW)
    {
        int64_t timestamp = sqlite3_column_int64(selectStmt, 0);
        uint8_t exceptionCode = static_cast<uint8_t>(sqlite3_column_int(selectStmt, 1));
        uint8_t expSensors = static_cast<uint8_t>(sqlite3_column_int(selectStmt, 2));
        uint8_t expGC31s = static_cast<uint8_t>(sqlite3_column_int(selectStmt, 3));

        std::cout << "Timestamp: " << timestamp
                  << ", Exception Code: " << static_cast<int>(exceptionCode)
                  << ", Exp Sensors: " << static_cast<int>(expSensors)
                  << ", Exp GC31s: " << static_cast<int>(expGC31s) << std::endl;
    }

    // 清理查询语句
    sqlite3_finalize(selectStmt);

    // return true;

    // 构建 SQL 删除语句
    std::string deleteSql = "DELETE FROM " + std::string(DEFAULT_DB_EXCEPTION_TABLE_NAME) +
                            " WHERE timestamp < " + std::to_string(expireTimestamp);

    char *errMsg = nullptr;
    // 执行删除操作
    if (sqlite3_exec(db, deleteSql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        std::cerr << "Failed to delete expired exception data: " << errMsg << std::endl;
        sqlite3_free(errMsg); // 释放错误信息
        return false;
    }

    std::cout << "Expired exception data deleted successfully." << std::endl;
    return true;
}
