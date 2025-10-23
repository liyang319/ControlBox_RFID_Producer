// DBWeighData.h

#ifndef DBWEIGHTDATA_H
#define DBWEIGHTDATA_H

#include "DB.h"
#include <vector>

typedef struct _OfflineDBDataUnit
{
    int timestamp;
    int handleflag;
    std::string weighInfo;
} OfflineDBDataUnit;

class DBWeighData : public DB
{
public:
    static DBWeighData &getInstance();
    ~DBWeighData();
    bool init();
    bool inserWeightData(int handleflag, int timestamp, std::string weighInfo);
    bool processAndDeleteData(int num, std::vector<OfflineDBDataUnit> &dataList);

private:
    const std::string m_tableName;
    DBWeighData(const std::string &dbPath);
    DBWeighData(const DBWeighData &) = delete;            // delete the copy constructor
    DBWeighData &operator=(const DBWeighData &) = delete; // delete the assignment operator
};

#endif