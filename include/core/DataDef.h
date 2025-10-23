#ifndef DATADEF_H
#define DATADEF_H

#include "Base.h"
#include <map>

#define MAX_WEIGH_DATA_QUEUE_SIZE 10
#define SENSOR_MAX_NUM 2
using namespace std;

#define EXCEPTION_REPORT

struct GPSData
{
    std::string time = "";
    double latitude = 0.0;
    double longitude = 0.0;
    double gps_height = 0.0;
    double gps_yaw = 0.0;
    int sv_num = 0;
    double gps_v = 0.0;
    double pdop = 0.0;
    double hdop = 0.0;
    double vdop = 0.0;
    GPSData() {}
};

// typedef struct _SensorSettingUnit
// {
//     unsigned char zeroCalib[SENSOR_MAX_NUM];
//     unsigned char gain[SENSOR_MAX_NUM];
//     unsigned char autoCalib[SENSOR_MAX_NUM];
//     unsigned char reverseFlag[SENSOR_MAX_NUM];
//     unsigned char filter[SENSOR_MAX_NUM];
//     unsigned char version;
// } SensorSettingUnit;

typedef struct _SensorSettingUnit
{
    uint16_t zeroCalib[SENSOR_MAX_NUM];
    uint16_t gain[SENSOR_MAX_NUM];
    uint16_t autoCalib[SENSOR_MAX_NUM];
    uint16_t reverseFlag[SENSOR_MAX_NUM];
    uint16_t filter[SENSOR_MAX_NUM];
    uint16_t version;
} SensorSettingUnit;

enum _MSENSOR_PROTOCOL_TYPE
{
    _MSENSOR_PROTOCOL_MSENSOR = 0,
    _MSENSOR_PROTOCOL_GC31,
    _MSENSOR_PROTOCOL_LOCATION,
    _MSENSOR_PROTOCOL_DTU_LOCATION,
    _WSENSOR_PROTOCL_4CHWEIGHT,
    _WSENSOR_PROTOCL_ZY4701,
    _SCREEN_PROTOCOL_DWIN = 0x80,
    _MSENSOR_PROTOCOL_NULL = 0xff,
};

typedef struct _SensorDataUnit
{
    unsigned char sensorAddr;
    uint16_t protocal;
    bool bActive = false;
    bool bSettingUpdated = true;
    int sensorID;
    int portID;
    std::string sensorKey;  // sensor01, sensor02
    std::string sensorName; // weight1, weight2
    std::string sensorSN;
    uint16_t chVal[SENSOR_MAX_NUM]; // GC31 TDA04D ZY4701
    double tempVal[SENSOR_MAX_NUM]; // GC31 ZY4701
    SensorSettingUnit settings;     // GC31 ZY4701
} SensorDataUnit;

typedef struct _WeighDataUnit
{
    uint16_t handleflag;
    map<uint8_t, SensorDataUnit> sensorDataMap;
    GPSData locationData;
} WeighDataUnit;

#define DEFAULT_WIN_STEP 1

#define DEFAULT_ZEROCALIB_VALUE 2480

//////////////////Mqtt消息定义
#define MSG_CMD_SET_FILTER "filter"
#define MSG_CMD_SET_EMPTYLOAD "zerocalib"
#define MSG_CMD_SET_GAIN "gaincalib"
#define MSG_CMD_SET_AUTOCALIB "autocalib"
#define MSG_CMD_SET_REVERSE "reverseflag"
#define MSG_CMD_RESET_DEVICE "reset"
#define MSG_CMD_REG_SET "registerset"
#define MSG_CMD_SHOW_WEIGHT "truckweight"
#define MSG_CMD_SET_CACULATE_MODE "modeset"
#define MSG_CMD_MODE_REPORT "modereport"
#define MSG_CMD_MODE_QUERY "mode"
#define MSG_CMD_RESTART_DEVICE "restart"
#define MSG_CMD_PARAMATERS "parameters"
#define MSG_CMD_PARAM_MODIF "modif"
#define MSG_CMD_PARAM_CALIBRATION "calibration"
#define MSG_CMD_PARAM_SENSOR "sensor"
#define MSG_CMD_PARAM_ALGORITHM "algorithm"
#define MSG_CMD_PARAM_INI "ini"
#define MSG_CMD_PARAM_JSON "json"
#define MSG_CMD_PARAM_DTU "dtu"
#define MSG_CMD_DFU "dfu"
#define MSG_CMD_CRT "crt"

#define MQTT_PUBLISH_TOPIC_INDEX_DATA 1
#define MQTT_PUBLISH_TOPIC_INDEX_RESPONSE 2
#define MQTT_PUBLISH_TOPIC_INDEX_OFFLINEDATA 3

typedef struct _MqttPublishUnit
{
    int topicIndex;
    std::string content;
} MqttPublishUnit;

//////////////////UI相关定义
#define UI_ADDR_NET_WEIGHT 0x2000
#define UI_ADDR_TARE_WEIGHT 0x2020
#define UI_ADDR_GROSS_WEIGHT 0x2002
#define UI_ADDR_VEHICLE_NO 0x3300
#define UI_ADDR_DEVICE_SN 0x3600

#define UI_ADDR_ZEROCALIB_CONFIRM 0x3800
#define UI_ADDR_TARE_CONFIRM 0x3810
#define UI_ADDR_EXCEPTION_MAIN 0x3820
#define UI_ADDR_EXCEPTION_MENU 0x3840
#define UI_ADDR_RESET_MENU 0x3850
#define UI_ADDR_TARE_INPUT 0x2500

#define UI_ADDR_LOCAL_MODE_SWITCH 0x1080
#define UI_ADDR_OFFLINE_MODE_SWITCH 0x1090
#define UI_ADDR_MAIN_ICON1 0x1020
#define UI_ADDR_MAIN_ICON2 0x1030
#define UI_ADDR_OVERLOAD_ICON 0x1001

#define UI_ADDR_EXCEPTION_PREVIOUS_PAGE 0x3880
#define UI_ADDR_EXCEPTION_NEXT_PAGE 0x3890

#define UI_ADDR_EXCEPTION_CONTENT_1 0x4000
#define UI_ADDR_EXCEPTION_CONTENT_2 0x4300
#define UI_ADDR_EXCEPTION_CONTENT_3 0x4600

#define UI_ADDR_DWIN_VERSION 0x3D00
#define UI_ADDR_MAIN_VERSION 0x3B00

#define UI_KEY_SWITCH_OFF 0x0000
#define UI_KEY_SWITCH_ON 0x0001

#define UI_KEY_OVERLOAD_ALARM_OFF 0x0001
#define UI_KEY_OVERLOAD_ALARM_ON 0x0000

#define UI_ICON_SWITCH_OFF 0x0007
#define UI_ICON_SWITCH_ON 0x0008

#define UI_KEY_ZEROCALIB 0x0001
#define UI_KEY_RESET 0x0003
#define UI_KEY_TARE_CONFIRM 0x0002
#define UI_KEY_EXCEPTION 0x0004
#define UI_KEY_EXCEPTION_PREVIOUS 0x0005
#define UI_KEY_EXCEPTION_NEXT 0x0006

#define UI_ICON_INDEX_OVERLOAD_ON 0x0000
#define UI_ICON_INDEX_OVERLOAD_OFF 0x0001
#define UI_ICON_INDEX_CLOUD_MODE_ON 0x0003
#define UI_ICON_INDEX_EDGE_MODE_ON 0x0004
#define UI_ICON_INDEX_OFFLINE_MODE_ON 0x0005
#define UI_ICON_INDEX_OFF 0x0006

#define UI_CMD_CODE_READ 0x83
#define UI_CMD_CODE_WRITE 0x82

#define UI_MSG_CMD_ZEROCALIB "zerocalib"
#define UI_MSG_CMD_RESET_DEVICE "reset"
#define UI_MSG_CMD_EXCETION_GET "exception_get"
#define UI_MSG_CMD_EXCETION_PREVIOUS "exception_previous"
#define UI_MSG_CMD_EXCETION_NEXT "exception_next"
#define UI_MSG_CMD_SET_TARE "settare"
#define UI_MSG_CMD_SET_LOCAL_MODE "setlocalmode"
#define UI_MSG_CMD_SET_OFFLINE_MODE "setofflinemode"

#define UI_CMD_UPDATE_WEIGHT "update_weight"
#define UI_CMD_OVERLOAD_ALARM_ON "overload_alarm_on"
#define UI_CMD_OVERLOAD_ALARM_OFF "overload_alarm_off"
#define UI_CMD_UPDATE_DWIN "update_dwin"
#define UI_CMD_SWITCH_PAGE "switch_page"

#define CAN_CMD_UPDATE_WEIGHT "update_weight"
#define CAN_CMD_ZEROCALIB_RESULT "zerocalib_result"

#define UI_DATA_FLOAT_SIZE 4
#define MIN_UI_MSG_LEN 9
#define DEFAULT_DWIN_DATA_BUFFER_SIZE 512
#define DEFAULT_EXCEPTIONS_UI_PAGE_SIZE 3
#define DEFAULT_EXCEPTIONS_UI_CONTENT_SIZE 100

#define UI_DWIN_PAGE_MAIN 0
#define UI_DWIN_PAGE_UPGRADING 6
/////////////////////////
#define DEFAULT_CUSTOM_CONFIG_PATH "./custom_config.ini"
#define CUSTOM_CONFIG_TAREWEIGHT "TareWeight"
#define CUSTOM_CONFIG_OFFLINE "offline"
#define CUSTOM_CONFIG_EDGE "edgecomputing"
#define DEFAULT_OTA_CONFIG_PATH "./ota_config.ini"
#define OTA_CONFIG_URL "url"
#define OTA_CONFIG_VERSION "version"
#define OTA_CONFIG_MD5 "md5"

//////////////GC31
#define DEFAULT_GC31_CMD_SIZE 30
#define DEFAULT_GC31_DATA_BUFFER_SIZE 30
#define MIN_SENSOR_RESPONSE_SIZE 5

typedef struct _SensorComUnit
{
    unsigned char addr;
    uint8_t protocal;
    uint8_t cmdType;
    uint8_t cmd[DEFAULT_GC31_DATA_BUFFER_SIZE] = {0};
    int len = 0;
} SensorComUnit;

typedef struct _SensorDataTimerUnit
{
    int sensorID;
    std::chrono::steady_clock::time_point lastTime;
} SensorDataTimerUnit;

//////////////Database
#define DEFAULT_DB_PATH "./weigh_data.db"
#define DEFAULT_DB_TABLE_NAME "weigh_data_offline"

#define DEFAULT_DB_EXCEPTION_PATH "./exception.db"
#define DEFAULT_DB_EXCEPTION_TABLE_NAME "exception_event"

//////////////ConfigFile
#define DEFAULT_CONFIG_FILE_PATH "sensor.ini"
#define DEFAULT_PARAMS_FILE_PATH "myprov.json"

/////////////Exception
#ifdef EXCEPTION_REPORT
// 异常类型
#define EXCEPTION_GC31_COM 1    // GC31通信异常
#define EXCEPTION_SENSOR_DATA 2 // 传感器数据异常
#define EXCEPTION_SERVER_COM 4  // 主机与云端通信异常
#define EXCEPTION_JSON_PARSE 8  // 主机解析JSON文件失败
#endif

//////////////Compute Mode

#define COMPUTE_MODE_CLOUD 0
#define COMPUTE_MODE_EDGE_SIMPLE 1
#define COMPUTE_MODE_EDGE_ALL 2

/////////////Can
#define CAN_MSG_TYPE_WEIGHT 1
#define CAN_MSG_TYPE_ZERO_RESULT 2
// xugong can message id
#define TX_XUGONG_CAN_MSG_ID 0x18FA0015
#define RX_XUGONG_CAN_MSG_ID 0x18FA0038

///////////////////////上报消息key定义
#define DEFAULT_DTU_VERSION "V0.1.33C"
#define DEFAULT_DTU_GC31_TYPE "GD32DTU-CG30"
#define DEFAULT_DTU_TDA04D_TYPE "GD32DTU-TDA04D"
#define DEFAULT_DTU_ZY4701_TYPE "GD32DTU-ZY4701"
#define DEFAULT_DTU_GYRO_TYPE "GD32DTU"
///////////////////////////标准字段key
#define DTU_REPORT_KEY_DTU "dtu"
#define DTU_REPORT_KEY_DTUSN "dtusn"
#define DTU_REPORT_KEY_VERSION "version"
#define DTU_REPORT_KEY_DTUTYPE "dtutype"
#define DTU_REPORT_KEY_SENSORSUM "sensorsum"
#define DTU_REPORT_KEY_HANDLEFLAG "handleflag"
#define DTU_REPORT_KEY_REPORTTIME "reporttime"
#define DTU_REPORT_KEY_WCAL "wcal"
#define DTU_REPORT_KEY_WEIGHT "weight"

#define DTU_REPORT_KEY_SENSORNAME "sensorname"
#define DTU_REPORT_KEY_LOCATION "location"
#define DTU_REPORT_KEY_CHIPTIME "chiptime"
#define DTU_REPORT_KEY_LON "lon"
#define DTU_REPORT_KEY_LAT "lat"
#define DTU_REPORT_KEY_GPSHEIGHT "gpsheight"
#define DTU_REPORT_KEY_GPSYAW "gpsyaw"
#define DTU_REPORT_KEY_GPSV "gpsv"
#define DTU_REPORT_KEY_SVNUM "svnum"
#define DTU_REPORT_KEY_PDOP "pdop"
#define DTU_REPORT_KEY_HDOP "hdop"
#define DTU_REPORT_KEY_VDOP "vdop"
#define DTU_REPORT_KEY_SENSOR "sensor"

#define DTU_REPORT_KEY_SENSORSN "sensorsn"
#define DTU_REPORT_KEY_CH1VAL "ch1val"
#define DTU_REPORT_KEY_CH2VAL "ch2val"
#define DTU_REPORT_KEY_TEMPVAL "tempval"
#define DTU_REPORT_KEY_ZEROCALIB "zerocalib"
#define DTU_REPORT_KEY_GAIN1 "gain1"
#define DTU_REPORT_KEY_GAIN2 "gain2"
#define DTU_REPORT_KEY_AUTOCALIB "autocalib"
#define DTU_REPORT_KEY_REVERSEFLAG "reverseflag"
#define DTU_REPORT_KEY_FILTER "filter"
#define DTU_REPORT_KEY_VERSIONGC "versiongc"
#define DTU_REPORT_KEY_EDGE "edge"
#define DTU_REPORT_KEY_FULLDATA "fulldata"

#define DTU_REPORT_TDA04D_KEY_CH1_FIRMVER "ch1_firmver"
#define DTU_REPORT_TDA04D_KEY_CH1_STATUS "ch1_status"
#define DTU_REPORT_TDA04D_KEY_CH1_WTMEASRT "ch1_wtmeasrt"
#define DTU_REPORT_TDA04D_KEY_CH1_NETWEIGHT "ch1_netweight"
#define DTU_REPORT_TDA04D_KEY_CH1_TAREWEIGHT "ch1_tareweigt"
#define DTU_REPORT_TDA04D_KEY_CH1_FULLSCALE "ch1_fullscale"
#define DTU_REPORT_TDA04D_KEY_CH1_SCALEDIV "ch1_scalediv"

#define DTU_REPORT_KEY_EXCETION "exception"
#define DTU_REPORT_KEY_EXSENSORS "exsensors"
#define DTU_REPORT_KEY_EXGC31S "exgc31s"
///////////////////////////压缩字段key
#define DTU_REPORT_COM_KEY_DTU "dtu"
#define DTU_REPORT_COM_KEY_DTUSN "sn"
#define DTU_REPORT_COM_KEY_VERSION "vs"
#define DTU_REPORT_COM_KEY_DTUTYPE "tp"
#define DTU_REPORT_COM_KEY_SENSORSUM "ss"
#define DTU_REPORT_COM_KEY_HANDLEFLAG "hf"
#define DTU_REPORT_COM_KEY_REPORTTIME "rt"
#define DTU_REPORT_COM_KEY_WCAL "wc"
#define DTU_REPORT_COM_KEY_WEIGHT "wt"

// #define DTU_REPORT_COM_KEY_SENSORNAME "sensorname"
#define DTU_REPORT_COM_KEY_LOCATION "location"
#define DTU_REPORT_COM_KEY_CHIPTIME "ct"
#define DTU_REPORT_COM_KEY_LON "lo"
#define DTU_REPORT_COM_KEY_LAT "la"
#define DTU_REPORT_COM_KEY_GPSHEIGHT "gh"
#define DTU_REPORT_COM_KEY_GPSYAW "gp"
#define DTU_REPORT_COM_KEY_GPSV "gv"
#define DTU_REPORT_COM_KEY_SVNUM "sc"
// #define DTU_REPORT_COM_KEY_PDOP "pdop"
// #define DTU_REPORT_COM_KEY_HDOP "hdop"
// #define DTU_REPORT_COM_KEY_VDOP "vdop"
#define DTU_REPORT_COM_KEY_SENSOR "sensor"

#define DTU_REPORT_COM_KEY_SENSORSN "sn"
#define DTU_REPORT_COM_KEY_CH1VAL "v1"
#define DTU_REPORT_COM_KEY_CH2VAL "v2"
#define DTU_REPORT_COM_KEY_TEMPVAL "tv"
// #define DTU_REPORT_COM_KEY_ZEROCALIB "zerocalib"
// #define DTU_REPORT_COM_KEY_GAIN1 "gain1"
// #define DTU_REPORT_COM_KEY_GAIN2 "gain2"
// #define DTU_REPORT_COM_KEY_AUTOCALIB "autocalib"
// #define DTU_REPORT_COM_KEY_REVERSEFLAG "reverseflag"
// #define DTU_REPORT_COM_KEY_FILTER "filter"
// #define DTU_REPORT_COM_KEY_VERSIONGC "versiongc"
#define DTU_REPORT_COM_KEY_EDGE "eg"
#define DTU_REPORT_COM_KEY_FULLDATA "fd"

#define DTU_REPORT_COM_KEY_EXCETION "ex"
#define DTU_REPORT_COM_KEY_EXSENSORS "es"
#define DTU_REPORT_COM_KEY_EXGC31S "ec"

#define DTU_REPORT_TDA04D_COM_KEY_CH1_FIRMVER "fv"
#define DTU_REPORT_TDA04D_COM_KEY_CH1_STATUS "st"
#define DTU_REPORT_TDA04D_COM_KEY_CH1_WTMEASRT "wt"
#define DTU_REPORT_TDA04D_COM_KEY_CH1_NETWEIGHT "nt"
#define DTU_REPORT_TDA04D_COM_KEY_CH1_TAREWEIGHT "tt"
#define DTU_REPORT_TDA04D_COM_KEY_CH1_FULLSCALE "fs"
#define DTU_REPORT_TDA04D_COM_KEY_CH1_SCALEDIV "sd"
////////////////////////////////////////////
// #define DEFAULT_DWIN_OTA_SAVE_PATH "/home/ubuntu/workdir/app/dwin_ota/"
#define DEFAULT_DWIN_OTA_SAVE_PATH "./dwin_ota/"
#define DEFAULT_LOGFILE_PREFIX "weighbox_"

#endif // DATADEF_H