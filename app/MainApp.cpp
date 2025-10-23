#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <mosquitto.h>
#include "Base.h"
#include "Version.h"
#include "GC31.h"

#include "MqttThread.h"
#include "MsgWorkerThread.h"
#include "DeviceConfig.h"
#include "ReportWorkerThread.h"
#include "CanManager.h"
#include "UIComThread.h"
#include "SensorComThread.h"
#include "Utility.h"
#include "GPS.h"
#include "Dwin.h"
#include "DBWeighData.h"
#include "OfflineDataReportThread.h"
#include "GlobalFlag.h"
#include "WatchDogThread.h"
#include "HttpUtility.h"
#include "DataReportThread.h"
#include "DBExceptionData.h"
#include "ExceptionDataProcessThread.h"
#include "GpsComThread.h"
#include "TDA04d.h"
#include "ZY4701.h"

// using namespace rapidjson;
// maeusing namespace std;

// #define SINGLE_INSTANCE_CHECK

int lockFile = -1; // 全局变量，保存文件描述符

bool isAlreadyRunning()
{
    const char *lockFilePath = "/var/run/WeighBox.lock";
    lockFile = open(lockFilePath, O_RDWR | O_CREAT, 0644);
    if (lockFile < 0)
    {
        std::cerr << "Failed to open lock file." << std::endl;
        return true;
    }
    // 使用 flock 加锁
    if (flock(lockFile, LOCK_EX | LOCK_NB) < 0)
    {
        close(lockFile);
        return true; // 无法加锁，说明已经有实例在运行
    }
    return false;
}

void init()
{
    Utility::initDevice();
    GlobalFlag::getInstance().init();
    GC31::getInstance().setEmptyLoadValue(0x31, 0x1234);
    // Utility::initDevice();
    // GlobalFlag::getInstance().init();
    // if (GlobalFlag::getInstance().sensorProtocol == _MSENSOR_PROTOCOL_GC31)
    // {
    //     GC31::getInstance().init();
    // }
    // else if (GlobalFlag::getInstance().sensorProtocol == _WSENSOR_PROTOCL_4CHWEIGHT)
    // {
    //     TDA04d::getInstance().init();
    // }
    // else if (GlobalFlag::getInstance().sensorProtocol == _WSENSOR_PROTOCL_ZY4701)
    // {
    //     ZY4701::getInstance().init();
    // }
    // GPS::getInstance().init();
    // WeighData::getInstance().init();
    // Dwin::getInstance().init();
    // DBWeighData::getInstance().init();
    // DBExceptionData::getInstance().init();
}

int main()
{
    COUT << "===========WeighBox==========" << VERSION << endl;
#ifdef SINGLE_INSTANCE_CHECK
    if (isAlreadyRunning())
    {
        COUT << "WeighBox is already running." << std::endl;
        return 1;
    }
#endif
    init();
    return 1;
#ifdef ENABLE_WATCHDOG
    WatchDogThread watchDogThread;
#endif
    MqttThread mqttThread;
    MsgWorkerThread workerThread;
    SensorComThread sensorComThread;
    GpsComThread gpsComThread;
    ReportWorkerThread reportWorkerThread;
    DataReportThread dataReportThread;
    OfflineDataReportThread offlineDataReportThread;
    CanManager &canManager = CanManager::getInstance();
    UIComThread uiComThread;
    ExceptionDataProcessThread exceptionDataProcessThread;

#ifdef ENABLE_WATCHDOG
    watchDogThread.start();
#endif
    mqttThread.start();
    workerThread.start();
    if (GlobalFlag::getInstance().sensorProtocol != _WSENSOR_PROTOCL_ZY4701)
    {
        sensorComThread.start();
    }
    gpsComThread.start();
    uiComThread.start();
    reportWorkerThread.start();
    dataReportThread.start();
    offlineDataReportThread.start();
    exceptionDataProcessThread.start();
    if (GlobalFlag::getInstance().bCan)
    {
        canManager.startLoop();
    }

    workerThread.join();
    mqttThread.join();
    sensorComThread.join();
    gpsComThread.join();
    reportWorkerThread.join();
    dataReportThread.join();
    offlineDataReportThread.join();
    canManager.stopGeneralDataReceive();
    exceptionDataProcessThread.join();
    uiComThread.join();
#ifdef ENABLE_WATCHDOG
    watchDogThread.join();
#endif

    return 0;
}
