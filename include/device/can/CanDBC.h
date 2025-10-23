#ifndef __CANDBC_H__
#define __CANDBC_H__

#include <vector>
#include <map>
#include "CanSock.h"
#include "CanFrame.h"

class CanDBC
{
    int messageId;

public:
    CanDBC();
    ~CanDBC();
    
    CanFrame* assembleFrame(int messageId, int canType);
    std::vector<CanSignal*>* parseFrame(CanFrame &frame);
};

#endif