// DBWeighData.cpp

#include "DBWeighData.h"
#include <string>
#include <iostream>
#include <unistd.h>
#include "DataDef.h"
#include "Utility.h"

DBWeighData::DBWeighData(const std::string &dbPath) : DB(dbPath) {}

DBWeighData &DBWeighData::getInstance()
{
    static DBWeighData instance(DEFAULT_DB_PATH);
    return instance;
}

bool DBWeighData::init()
{
    if (!open())
    {
        std::cout << "Failed to open database." << std::endl;
        return false;
    }
    std::string columns = "timestamp INTEGER PRIMARY KEY, handleflag INTEGER, weigh_info TEXT";
    if (createTable(DEFAULT_DB_TABLE_NAME, columns))
    {
        std::cout << "Table weigh_data created successfully." << std::endl;
    }
    else
    {
        std::cout << "Failed to create table weigh_data." << std::endl;
    }
    return true;
}

bool DBWeighData::inserWeightData(int handleflag, int timestamp, std::string weighInfo)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    std::string values = std::to_string(timestamp) + ", " + std::to_string(handleflag) + ", '" + weighInfo + "'";
    insertData(DEFAULT_DB_TABLE_NAME, values);
    return true;
}

DBWeighData::~DBWeighData()
{
    close();
}

// bool DBWeighData::processAndDeleteData()
// {
//     int index = 0;
//     while (!isDbEmpty(DEFAULT_DB_TABLE_NAME))
//     {
//         lockDB();

//         // Select 10 rows from the table
//         std::string selectSql = "SELECT * FROM " + std::string(DEFAULT_DB_TABLE_NAME) + " LIMIT 10;";
//         sqlite3_stmt *stmt;
//         if (sqlite3_prepare_v2(getDB(), selectSql.c_str(), -1, &stmt, NULL) != SQLITE_OK)
//         {
//             std::cerr << "Error preparing select statement." << std::endl;
//             unlockDB();
//             break;
//         }
//         std::cout << "--------------------" << index++ << std::endl;
//         while (sqlite3_step(stmt) == SQLITE_ROW)
//         {
//             int timestamp = sqlite3_column_int(stmt, 0);
//             int handleflag = sqlite3_column_int(stmt, 1);
//             const unsigned char *weighInfo = sqlite3_column_text(stmt, 2);
//             // Output the data
//             std::cout << "Timestamp: " << timestamp << ", Handleflag: " << handleflag << ", WeighInfo: " << weighInfo << std::endl;
//         }

//         sqlite3_finalize(stmt);

//         // Delete the 10 rows from the table
//         std::string deleteSql = "DELETE FROM " + std::string(DEFAULT_DB_TABLE_NAME) + " WHERE timestamp IN (SELECT timestamp FROM " + DEFAULT_DB_TABLE_NAME + " LIMIT 10);";
//         if (!execute(deleteSql))
//         {
//             std::cerr << "Error deleting data." << std::endl;
//             unlockDB();
//             break;
//         }

//         unlockDB();
//     }

//     return true;
// }

// void DBWeighData::processAndDeleteData(int num, std::vector<OfflineDBDataUnit> &dataList)
// {
//     int count = 0;
//     while (count < num)
//     {
//         lockDB();

//         // Select 1 row from the table
//         std::string selectSql = "SELECT * FROM " + std::string(DEFAULT_DB_TABLE_NAME) + " LIMIT 1;";
//         sqlite3_stmt *stmt;
//         if (sqlite3_prepare_v2(getDB(), selectSql.c_str(), -1, &stmt, NULL) != SQLITE_OK)
//         {
//             std::cerr << "Error preparing select statement." << std::endl;
//             unlockDB();
//             break;
//         }

//         // Process the data here
//         if (sqlite3_step(stmt) == SQLITE_ROW)
//         {
//             OfflineDBDataUnit dataUnit;
//             dataUnit.timestamp = sqlite3_column_int(stmt, 0);
//             dataUnit.handleflag = sqlite3_column_int(stmt, 1);
//             dataUnit.weighInfo = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

//             // Add the dataUnit to the dataList
//             dataList.push_back(dataUnit);

//             // Increment count
//             count++;

//             // Delete the row from the table
//             std::string deleteSql = "DELETE FROM " + std::string(DEFAULT_DB_TABLE_NAME) + " WHERE timestamp = " + std::to_string(dataUnit.timestamp) + ";";
//             if (!execute(deleteSql)) {
//                 std::cerr << "Error deleting data." << std::endl;
//                 unlockDB();
//                 break;
//             }
//         }
//         else
//         {
//             // No more data in the database, break out of the loop
//             break;
//         }

//         sqlite3_finalize(stmt);

//         unlockDB();
//     }
// }

bool DBWeighData::processAndDeleteData(int num, std::vector<OfflineDBDataUnit> &dataList)
{
    bool success = true;
    int count = 0;
    if (isDbEmpty(DEFAULT_DB_TABLE_NAME))
    {
        return true;
    }
    std::lock_guard<std::mutex> lock(db_mutex);
    while (count < num)
    {
        // lockDB();
        // Select 1 row from the table
        std::string selectSql = "SELECT * FROM " + std::string(DEFAULT_DB_TABLE_NAME) + " LIMIT 1;";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(getDB(), selectSql.c_str(), -1, &stmt, NULL) != SQLITE_OK)
        {
            std::cerr << "Error preparing select statement." << std::endl;
            // unlockDB();
            success = false;
            break;
        }
        // Process the data here
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            OfflineDBDataUnit dataUnit;
            dataUnit.timestamp = sqlite3_column_int(stmt, 0);
            dataUnit.handleflag = sqlite3_column_int(stmt, 1);
            dataUnit.weighInfo = std::string(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2)));
            // Add the dataUnit to the dataList
            dataList.push_back(dataUnit);

            // Increment count
            count++;
            // Delete the row from the table
            std::string deleteSql = "DELETE FROM " + std::string(DEFAULT_DB_TABLE_NAME) + " WHERE timestamp = " + std::to_string(dataUnit.timestamp) + ";";
            if (!execute(deleteSql))
            {
                std::cerr << "Error deleting data." << std::endl;
                // unlockDB();
                success = false;
                break;
            }
        }
        else
        {
            // No more data in the database, break out of the loop
            break;
        }
        sqlite3_finalize(stmt);
        // unlockDB();
    }

    return success;
}