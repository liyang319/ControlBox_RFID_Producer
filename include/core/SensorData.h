
#ifndef SENSORDATA_H
#define SENSORDATA_H

#include <queue>
#include <array>
#include <mutex>
#include "DataDef.h"


class SensorData
{

private:
    SensorData();                                       // private constructor to prevent instantiation
    SensorData(const SensorData &) = delete;            // delete the copy constructor
    SensorData &operator=(const SensorData &) = delete; // delete the assignment operator

    std::queue<SensorComUnit> data_recv_queue;
    std::queue<SensorComUnit> data_send_queue;

    std::mutex data_recv_queue_mutex;
    std::mutex data_send_queue_mutex;

public:
    static SensorData &getInstance();

    void addDataToDataRecvQueue(SensorComUnit &data);

    SensorComUnit getDataFromDataRecvQueue();

    void addDataToDataSendQueue(SensorComUnit &data);

    SensorComUnit getDataFromDataSendQueue();

    int getDataRecvQueueSize() { return data_recv_queue.size(); };

    int getDataSendQueueSize() { return data_send_queue.size(); };
};

#endif // APPDATA_H
