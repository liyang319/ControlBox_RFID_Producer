#ifndef MSGDISPATCHER_H
#define MSGDISPATCHER_H

#include "rapidjson/document.h"
#include <string>
#include "DataDef.h"

class MsgDispatcher
{
public:
    MsgDispatcher(std::string &json_data);

    void dispatchMsg();

private:
    void processSetFilterCmd();
    void processSetEmptyLoadValueCmd();
    void processSetGainCmd();
    void processSetReverseCmd();
    void processResetDeviceCmd();
    void processRestartDeviceCmd();
    void processSetAutoCalibrationCmd();
    void processSetCaculateModeCmd();
    void processShowWeightCmd();
    bool getSensorAddr();
    void processModeQueryCmd();
    void processGetModifParamCmd();
    void processGetCalibrationParamCmd();
    void processGetSensorParamCmd();
    void processGetAlgorithmParamCmd();
    void processGetDtuParamCmd();
    void processDfuCmd();
    void processGetIni();
    void processGetJson();
    void processCrtCmd();

    rapidjson::Document m_document;
    std::string sensorKey;
    unsigned char sensorAddr;
    int sensorID;
};

#endif // MSGDISPATCHER_H
