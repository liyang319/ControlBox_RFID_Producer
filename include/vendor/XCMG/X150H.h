#ifndef _X150H_H_
#define _X150H_H_

// xugong can message id
#define TX_XUGONG_CAN_MSG_ID  0x18FA0015
#define RX_XUGONG_CAN_MSG_ID  0x18FA0038

struct WeighCanData
{
    unsigned short axls1;           // byte 0
    unsigned short axls2;           // byte 1
    unsigned short axls3;           // byte 2
    unsigned char error;            // byte 3
    unsigned char errorSensorid;    // byte 4
    bool canlibrated;               // byte 5
    double weigh;                   // byte 6&7
};

class X150H
{
private:
    
public:
    X150H(/* args */);
    ~X150H();

    // received cmd from CAN bus
    bool ZeroCalibration(unsigned short sensorid);

    // send data to CAN bus
    bool SendCanData(WeighCanData data);
};

#endif 