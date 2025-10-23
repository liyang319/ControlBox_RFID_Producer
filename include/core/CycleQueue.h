#ifndef CYCLEQUEUE_H
#define CYCLEQUEUE_H

#include <deque>
#include <mutex>
#include "DataDef.h"

#define MAX_QUEUE_SIZE 100

class CycleQueue
{
private:
    std::deque<WeighDataUnit> circularList;
    int frontIndex;
    int rearIndex;
    std::mutex mtx;

public:
    CycleQueue();
    void addDataToQueue(const WeighDataUnit &data);
    std::deque<WeighDataUnit> getDataFromQueue(int data_num);
    void removeDataFromQueue(int data_num);
    int getDataQueueSize();
    void printDataQueue();
    void clear();
};

#endif
