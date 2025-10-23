// appdata.h
#ifndef UIDATA_H
#define UIDATA_H

#include <queue>
#include <array>
#include <mutex>
#include "DataDef.h"

#define DEFAULT_UI_DATA_SIZE 100

typedef struct _UIDataUnit
{
    std::string cmd;
    unsigned char *content;
    int len;
    _UIDataUnit() : cmd(""), content(nullptr), len(0) {}
} UIDataUnit;

class UIData
{

private:
    UIData();                                   // private constructor to prevent instantiation
    UIData(const UIData &) = delete;            // delete the copy constructor
    UIData &operator=(const UIData &) = delete; // delete the assignment operator

    std::queue<UIDataUnit> data_recv_queue;
    std::queue<UIDataUnit> data_send_queue;

    std::mutex data_recv_queue_mutex;
    std::mutex data_send_queue_mutex;

public:
    static UIData &getInstance();

    void addDataToDataRecvQueue(UIDataUnit &data);

    UIDataUnit getDataFromDataRecvQueue();

    void addDataToDataSendQueue(UIDataUnit &data);

    UIDataUnit getDataFromDataSendQueue();

    int getDataRecvQueueSize() { return data_recv_queue.size(); };

    int getDataSendQueueSize() { return data_send_queue.size(); };
};

#endif // UIDATA_H
