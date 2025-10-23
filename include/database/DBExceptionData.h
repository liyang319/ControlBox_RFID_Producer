#ifndef DBEXCEPTIONDATA_H
#define DBEXCEPTIONDATA_H

#include "DB.h"
#include <vector>

typedef struct _ExceptionDBDataUnit
{
    int timestamp;
    uint8_t exceptionCode;
    uint8_t expSensors; // 传感器数据异常时的传感器位置
    uint8_t expGC31s;
} ExceptionDBDataUnit;

class DBExceptionData : public DB
{
public:
    static DBExceptionData &getInstance();
    ~DBExceptionData();
    bool init();
    bool inserExceptionData();
    bool checkExceptionData();
    bool loadExceptionsFromDB();
    bool getExceptions(int pageIndex, std::vector<ExceptionDBDataUnit> &vecExp, int pageSize);
    int currentExpPageIndex;
    std::vector<ExceptionDBDataUnit> &getExceptions() { return exceptionVec; }
    int getPageNum() { return pageNum; };
    bool deleteExpireExceptionData(int hours);
    // bool processAndDeleteData(int num, std::vector<OfflineDBDataUnit> &dataList);

private:
    ExceptionDBDataUnit currentException;
    std::vector<ExceptionDBDataUnit> exceptionVec;
    int pageNum;

    const std::string m_tableName;
    DBExceptionData(const std::string &dbPath);
    DBExceptionData(const DBExceptionData &) = delete;            // delete the copy constructor
    DBExceptionData &operator=(const DBExceptionData &) = delete; // delete the assignment operator
};

#endif // DBEXCEPTIONDATA_H