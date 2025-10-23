#include "UIData.h"

UIData &UIData::getInstance()
{
    static UIData instance;
    return instance;
}

void UIData::addDataToDataRecvQueue(UIDataUnit &data)
{
    std::lock_guard<std::mutex> lock(data_recv_queue_mutex);
    data_recv_queue.push(data);
}

UIDataUnit UIData::getDataFromDataRecvQueue()
{
    std::lock_guard<std::mutex> lock(data_recv_queue_mutex);
    UIDataUnit data;
    if (!data_recv_queue.empty())
    {
        data = data_recv_queue.front();
        data_recv_queue.pop();
    }
    return data;
}

void UIData::addDataToDataSendQueue(UIDataUnit &data)
{
    std::lock_guard<std::mutex> lock(data_send_queue_mutex);
    data_send_queue.push(data);
}

UIDataUnit UIData::getDataFromDataSendQueue()
{
    std::lock_guard<std::mutex> lock(data_send_queue_mutex);
    UIDataUnit data;
    if (!data_send_queue.empty())
    {
        data = data_send_queue.front();
        data_send_queue.pop();
    }
    return data;
}

UIData::UIData()
{
    // constructor
}
