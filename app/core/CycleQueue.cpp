#include "CycleQueue.h"

CycleQueue::CycleQueue() : frontIndex(0), rearIndex(0)
{
    circularList.clear();
}

void CycleQueue::addDataToQueue(const WeighDataUnit &data)
{
    std::lock_guard<std::mutex> lock(mtx);
    circularList.push_back(data); // 在队列尾部插入数据
    if (circularList.size() > MAX_QUEUE_SIZE)
    {
        circularList.pop_front(); // 如果队列大小超过最大值，弹出队列头部数据
    }
}

std::deque<WeighDataUnit> CycleQueue::getDataFromQueue(int data_num)
{
    std::lock_guard<std::mutex> lock(mtx);
    std::deque<WeighDataUnit> result;
    if (data_num <= circularList.size())
    {
        for (int i = 0; i < data_num; i++)
        {
            result.push_back(circularList[i]);
        }
    }
    else
    {
        // 如果请求的数据数目超过队列大小，则返回全部数据
        result = circularList;
    }
    return result;
}

void CycleQueue::removeDataFromQueue(int data_num)
{
    std::lock_guard<std::mutex> lock(mtx);
    // 删除最先插入的data_num个数据
    if (data_num >= circularList.size())
    {
        // 清空整个队列
        circularList.clear();
    }
    else
    {
        // 删除最先插入的data_num个数据
        for (int i = 0; i < data_num; i++)
        {
            circularList.pop_front();
        }
    }
}

int CycleQueue::getDataQueueSize()
{
    std::lock_guard<std::mutex> lock(mtx);
    return circularList.size();
}

void CycleQueue::printDataQueue()
{
    std::lock_guard<std::mutex> lock(mtx);
    for (int i = 0; i < circularList.size(); i++)
    {
        COUT << circularList[i].locationData.time << std::endl;
    }
}

void CycleQueue::clear()
{
    std::lock_guard<std::mutex> lock(mtx);
    circularList.clear();
}
