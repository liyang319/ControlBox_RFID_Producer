#include "CanData.h"

CanData &CanData::getInstance()
{
    static CanData instance;
    return instance;
}

void CanData::addDataToDataRecvQueue(CanDataUnit &data)
{
    std::lock_guard<std::mutex> lock(data_recv_queue_mutex);
    data_recv_queue.push(data);
}

CanDataUnit CanData::getDataFromDataRecvQueue()
{
    std::lock_guard<std::mutex> lock(data_recv_queue_mutex);
    CanDataUnit data;
    if (!data_recv_queue.empty())
    {
        data = data_recv_queue.front();
        data_recv_queue.pop();
    }
    return data;
}

void CanData::addDataToDataSendQueue(CanDataUnit &data)
{
    std::lock_guard<std::mutex> lock(data_send_queue_mutex);
    data_send_queue.push(data);
}

CanDataUnit CanData::getDataFromDataSendQueue()
{
    std::lock_guard<std::mutex> lock(data_send_queue_mutex);
    CanDataUnit data;
    if (!data_send_queue.empty())
    {
        data = data_send_queue.front();
        data_send_queue.pop();
    }
    return data;
}

CanData::CanData()
{
    // constructor
}
