#include "CanDBC.h"
#include "Base.h"

using namespace std;

CanDBC::CanDBC()
{
    
}

CanDBC::~CanDBC()
{
}

/**
 * 将多个信号编码入一个CAN帧
 * messageId: 帧Id
 * canType: CAN数据帧类型
 */
CanFrame* CanDBC::assembleFrame(int messageId, int canType)
{
    CanFrame* canFrame = new CanFrame(canType);
    this->messageId = messageId;

    // init frame
    canFrame->id = messageId;
    canFrame->length = 8;//first->MessageLen;       // 从sql中获取？还是每次调用时指定？
    canFrame->isExtend = 1;//first->StdOrExtFlg;    // 从sql中获取？还是每次调用时指定？
    canFrame->isRemote = false;

    // data frame
    if (canType == CAN_TYPE_CAN_20)
    {
        if (canFrame->length > FRAME_MAX_DLC)
        {
            canFrame->length = FRAME_MAX_DLC;
        }
    }
    else
    {
        if (canFrame->length > FRAME_MAX_FD_DLC)
        {
            canFrame->length = FRAME_MAX_FD_DLC;
        }
    }

    // insert signal
    /*
    for (unsigned int i=0; i<dbcs.size(); i++)
    {
        CanSignal signal;
        signal.id = item->SignalIndex;
        signal.bitSize = item->BitLen;
        signal.byteOrder = item->ByteOrder;
        signal.startBit = item->StartBit;
        signal.binding = item->Binding;

        // TODO 通过signal id & binding，找到对应的lua全局变量，给signal付值
        signal.value = 0x54+i;

        canFrame->insertSignal(signal);
    }

    // 清除数据
    for (unsigned int i=0; i<dbcs.size(); i++)
    {
        Table_CAN_DBC* item = dbcs.at(i);
        delete item;
    }

    dbcs.clear(); 
    */

    return canFrame;
}

/**
 * 从一个CAN帧，解析出所有的信号数据
 * messageId: 帧Id
 * canType: CAN数据帧类型
 * std::vector<CanSignal*>: 返回信号数组
 */
std::vector<CanSignal*>* CanDBC::parseFrame(CanFrame &frame)
{
    std::vector<CanSignal*> *signals = new std::vector<CanSignal*>();
    this->messageId = frame.id;

    /*
    this->getDbcs();

    if (dbcs.size() == 0)
    {
        COUT << "Invalid parameters!" << endl;
        return NULL;
    }

    // dbc
    for (unsigned int i=0; i<dbcs.size(); i++)
    {
        Table_CAN_DBC* item = dbcs.at(i);
        CanSignal* signal = frame.getSignal(item->StartBit, item->BitLen, item->ByteOrder);
        signal->id = item->SignalIndex;
        signal->binding = item->Binding;
        signals->push_back(signal);

        // TODO 通过signal id & binding，找到对应的lua全局变量，给该lua变量付值

    }
    */

    return signals;
}