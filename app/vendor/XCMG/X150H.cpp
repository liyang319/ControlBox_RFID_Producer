#include "X150H.h"
#include "Base.h"
#include "CanManager.h"
#include "MdtuProv.h"

X150H::X150H(/* args */)
{
    COUT << "X150H init!" << endl;
}

X150H::~X150H()
{
    COUT << "X150H destroy!" << endl;
}

/**
 * @brief 置零命令字符串
 * 
 * @return true 
 * @return false 
 */
bool X150H::ZeroCalibration(unsigned short sensorid)
{
    // 调用发送接口

    return true;
}

/**
 * @brief 给主机上报重量及故障信息
 * 
 * @param data 
 * @return true 
 * @return false 
 */
bool X150H::SendCanData(WeighCanData data)
{
    GeneralCanData can;

    can.isExtend = true;
    can.messageId = TX_XUGONG_CAN_MSG_ID;
    can.length = 8;

    can.data[0] = data.axls1;
    can.data[1] = data.axls2;
    can.data[2] = data.axls3;
    can.data[3] = data.error;
    can.data[4] = data.errorSensorid;

    MdtuProv &prov = MdtuProv::getInstance();
    can.data[5] = prov.isParsed()?1:0;

    unsigned short w = data.weigh / 100;  // 公斤 -> 0.1吨
    can.data[6] = w & 0xFF;
    can.data[7] = (w>>8) & 0xFF;

    CanManager &canManager = CanManager::getInstance();
    canManager.sendGeneralData(can);

    return true;
}
