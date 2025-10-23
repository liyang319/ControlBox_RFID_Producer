#ifndef UIMSGDISPATCHER_H
#define UIMSGDISPATCHER_H

#include "rapidjson/document.h"
#include <string>

// #define MSG_CMD_SET_FILTER "filter"
// #define MSG_CMD_SET_EMPTYLOAD "zerocalib"
// #define MSG_CMD_SET_GAIN "gaincalib"
// #define MSG_CMD_SET_AUTOCALIB "autocalib"
// #define MSG_CMD_SET_REVERSE "reverseflag"
// #define MSG_CMD_RESET_DEVICE "reset"

class UIMsgDispatcher
{
public:
    UIMsgDispatcher();

    void dispatchMsg(uint8_t *data, int len);

private:
    void processZeroCalibCmd();
    void processResetDeviceCmd();
    void processExceptionCmd();
    void processSetTareCmd();
    void processExceptionPreviousCmd();
    void processExceptionNextCmd();

    void processSetLocalMode(uint8_t *data, int len);
    void processSetOfflineMode(uint8_t *data, int len);

    uint16_t msg_addr;
    uint16_t msg_code;
};

#endif // UIMSGDISPATCHER_H
