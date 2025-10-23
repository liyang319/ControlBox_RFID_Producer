// appdata.h
#ifndef APPDATA_H
#define APPDATA_H

#include <queue>
#include <array>
#include <mutex>
#include "DataDef.h"

class AppData
{

private:
    AppData();                                    // private constructor to prevent instantiation
    AppData(const AppData &) = delete;            // delete the copy constructor
    AppData &operator=(const AppData &) = delete; // delete the assignment operator

    std::queue<std::string> data_recv_queue;
    std::queue<MqttPublishUnit> data_send_queue;

    std::mutex data_recv_queue_mutex;
    std::mutex data_send_queue_mutex;

public:
    static AppData &getInstance();

    void addDataToDataRecvQueue(std::string &data);

    std::string getDataFromDataRecvQueue();

    void addDataToDataSendQueue(MqttPublishUnit &data);

    MqttPublishUnit getDataFromDataSendQueue();

    int getDataRecvQueueSize() { return data_recv_queue.size(); };

    int getDataSendQueueSize() { return data_send_queue.size(); };
};

#endif // APPDATA_H
