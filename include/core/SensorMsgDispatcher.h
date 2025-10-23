#ifndef SENSORMSGDISPATCHER_H
#define SENSORMSGDISPATCHER_H

#include <string>
#include "DataDef.h"

class SensorMsgDispatcher
{
public:
    SensorMsgDispatcher();

    void dispatchMsg(uint8_t *data, int len);

    void dispatchMsg(SensorComUnit sensorComUnit);

    bool getSensorParam(uint8_t *data, int len, uint8_t &addr, uint8_t &cmdType);

private:
    /////////GC31 functions////////////////////////
    void processGetSNCmd(uint8_t *data, int len);
    void processReadDataCmd(uint8_t *data, int len);
    void processSetFilterCmd(uint8_t *data, int len);
    void processSetEmptyloadCmd(uint8_t *data, int len);
    void processSetGainCmd(uint8_t *data, int len);
    void processSetReverseCmd(uint8_t *data, int len);
    void processGetSettingCmd(uint8_t *data, int len);
    void processResetDeviceCmd(uint8_t *data, int len);
    void processGetVersionCmd(uint8_t *data, int len);
    void processSetAutocalibrationCmd(uint8_t *data, int len);
    /////////TDA04d functions////////////////////////
    void processReadDataCmd_TAD04d(uint8_t *data, int len);

    uint8_t msg_addr;
    uint8_t msg_cmdType;
};

#endif // SensorMsgDispatcher
