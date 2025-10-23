// appdata.h
#ifndef CANDATA_H
#define CANDATA_H

#include <queue>
#include <array>
#include <mutex>
#include "DataDef.h"

#define DEFAULT_CAN_DATA_SIZE 100

typedef struct _CanDataUnit
{
    std::string cmd;
    unsigned char *content;
    int len;
    _CanDataUnit() : cmd(""), content(nullptr), len(0) {}
} CanDataUnit;

class CanData
{

private:
    CanData();                                    // private constructor to prevent instantiation
    CanData(const CanData &) = delete;            // delete the copy constructor
    CanData &operator=(const CanData &) = delete; // delete the assignment operator

    std::queue<CanDataUnit> data_recv_queue;
    std::queue<CanDataUnit> data_send_queue;

    std::mutex data_recv_queue_mutex;
    std::mutex data_send_queue_mutex;

public:
    static CanData &getInstance();

    void addDataToDataRecvQueue(CanDataUnit &data);

    CanDataUnit getDataFromDataRecvQueue();

    void addDataToDataSendQueue(CanDataUnit &data);

    CanDataUnit getDataFromDataSendQueue();

    int getDataRecvQueueSize() { return data_recv_queue.size(); };

    int getDataSendQueueSize() { return data_send_queue.size(); };
};

#endif // CANDATA_H
