#include "SensorData.h"

SensorData &SensorData::getInstance()
{
    static SensorData instance;
    return instance;
}

void SensorData::addDataToDataRecvQueue(SensorComUnit &data)
{
    std::lock_guard<std::mutex> lock(data_recv_queue_mutex);
    data_recv_queue.push(data);
}

SensorComUnit SensorData::getDataFromDataRecvQueue()
{
    std::lock_guard<std::mutex> lock(data_recv_queue_mutex);
    SensorComUnit data;
    if (!data_recv_queue.empty())
    {
        data = data_recv_queue.front();
        data_recv_queue.pop();
    }
    return data;
}

void SensorData::addDataToDataSendQueue(SensorComUnit &data)
{
    std::lock_guard<std::mutex> lock(data_send_queue_mutex);
    data_send_queue.push(data);
}

SensorComUnit SensorData::getDataFromDataSendQueue()
{
    std::lock_guard<std::mutex> lock(data_send_queue_mutex);
    SensorComUnit data;
    if (!data_send_queue.empty())
    {
        data = data_send_queue.front();
        data_send_queue.pop();
    }
    return data;
}

SensorData::SensorData()
{
    // constructor
}
