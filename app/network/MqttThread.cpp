#include "MqttThread.h"
#include "Base.h"
#include <unistd.h>
#include "AppData.h"
#include "MqttReportThread.h"
#include "DeviceConfig.h"
#include "Utility.h"
#include "GlobalFlag.h"
#include "WeighData.h"
#include "DBExceptionData.h"

bool MqttThread::isRunning = false;
bool MqttThread::isConnected = false;
std::string MqttThread::mqttSubscribeTopic = "";

MqttThread::MqttThread() : reportThread(this)
{
    init();
    mosquitto_lib_init();
    mosq = mosquitto_new(mqttClinetID.c_str(), true, NULL);
    if (!mosq)
    {
        std::cerr << "创建mosquitto客户端失败" << std::endl;
        return;
    }

    mosquitto_message_callback_set(mosq, message_callback);
    mosquitto_connect_callback_set(mosq, connect_callback);
    mosquitto_disconnect_callback_set(mosq, disconnect_callback);
}

void MqttThread::init()
{
    mqttHost = DeviceConfig::getInstance().get_value("dtu", "mqtthost");
    mqttPort = DeviceConfig::getInstance().get_value("dtu", "mqttport");
    mqttUsername = DeviceConfig::getInstance().get_value("dtu", "mqttusername");
    mqttPassword = DeviceConfig::getInstance().get_value("dtu", "mqttpassword");
    mqttClinetID = "MDTU_" + Utility::getDeviceSN();
    mqttPublishDataTopic = "DEVICE/DATA/DTU/" + Utility::getDeviceSN();
    mqttPublishResponseTopic = "DEVICE/RESPONSE/DTU/" + Utility::getDeviceSN();
    mqttPublishOfflineDataTopic = "DEVICE/OFFLINEDATA/DTU/" + Utility::getDeviceSN();
    mqttSubscribeTopic = "DEVICE/COMMAND/DTU/" + Utility::getDeviceSN();
    if (GlobalFlag::getInstance().bMqttDataLogOn)
    {
        std::cout << "mqttClinetID       :  " << mqttClinetID << std::endl;
        std::cout << "mqttPublishDataTopic   :  " << mqttPublishDataTopic << std::endl;
        std::cout << "mqttPublishResponseTopic   :  " << mqttPublishResponseTopic << std::endl;
        std::cout << "mqttSubscribeTopic :  " << mqttSubscribeTopic << std::endl;
    }
}

MqttThread::~MqttThread()
{
    stop();
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
}

void MqttThread::start()
{
    if (!isRunning)
    {
        isRunning = true;
        thread = std::thread([this]
                             {
            while (isRunning)
            {
                // 设置用户名和密码
                mosquitto_username_pw_set(mosq, mqttUsername.c_str(), mqttPassword.c_str());
                if (!isConnected && mosquitto_connect(mosq, mqttHost.c_str(), stoi(mqttPort), MQTT_CONNECT_TIMEOUT))
                {
                    COUT << "连接到MQTT服务器失败，尝试重连" << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(5)); // 等待5秒后重连
                }
                else
                {
                    // if (!isConnected && mosquitto_subscribe(mosq, NULL, this->mqttSubscribeTopic.c_str(), 0))
                    // {
                    //     COUT << "订阅" << this->mqttSubscribeTopic << "失败" << std::endl;
                    //     return;
                    // }
                    if (!isConnected)
                    {
                        //isConnected = true;
                        mosquitto_loop_start(mosq);
                    }
                    // mosquitto_loop_start(mosq);
                }
                sleep(5);
            } });
        reportThread.start(); // 启动MqttReportThread
    }
}

void MqttThread::stop()
{
    if (isRunning)
    {
        isRunning = false;
        reportThread.stop(); // 停止MqttReportThread
        reportThread.join(); // 等待reportThread完成
        mosquitto_disconnect(mosq);
        if (thread.joinable())
        {
            thread.join();
        }
    }
}

void MqttThread::join()
{
    if (thread.joinable())
    {
        thread.join();
    }
}

void MqttThread::message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
    if (message->payloadlen)
    {
        string recvData = (char *)message->payload;
        std::cout << "收到消息: " << recvData << std::endl;
        AppData::getInstance().addDataToDataRecvQueue(recvData);
    }
    else
    {
        std::cout << "收到空消息" << std::endl;
    }
}

void MqttThread::connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
    if (!result)
    {
        isConnected = true;
        GlobalFlag::getInstance().bDataCanReport = true;
        COUT << "连接成功" << std::endl;
        // 重新订阅需要订阅的主题
        if (mosquitto_subscribe(mosq, NULL, mqttSubscribeTopic.c_str(), 0))
        {
            COUT << "重新订阅" << mqttSubscribeTopic << "失败" << std::endl;
            return;
        }
#ifdef EXCEPTION_REPORT
        GlobalFlag::getInstance().exceptionCode = GlobalFlag::getInstance().exceptionCode & (~EXCEPTION_SERVER_COM);
#endif
        // 启动MQTT客户端的主循环
        mosquitto_loop_start(mosq);
    }
    else
    {
        isConnected = false;
        COUT << "连接失败" << std::endl;
    }
}

void MqttThread::disconnect_callback(struct mosquitto *mosq, void *userdata, int result)
{
    COUT << "mqtt连接断开，尝试重新连接" << std::endl;
    isConnected = false;
    GlobalFlag::getInstance().bDataCanReport = false;
#ifdef EXCEPTION_REPORT
    GlobalFlag::getInstance().exceptionCode = GlobalFlag::getInstance().exceptionCode | EXCEPTION_SERVER_COM;
    DBExceptionData::getInstance().inserExceptionData();
#endif
    // std::this_thread::sleep_for(std::chrono::seconds(5)); // 等待5秒后重连

    // while (isRunning && mosquitto_reconnect(mosq))
    // {
    //     std::cerr << "重连失败，等待5秒后重试" << std::endl;
    //     std::this_thread::sleep_for(std::chrono::seconds(10)); // 等待5秒后重连
    // }
}

void MqttThread::publish(const std::string &topic, const std::string &payload, int qos, bool retain)
{
    if (!isConnected)
    {
        COUT << "Mqtt disconnected! The messenge cant't be published." << endl;
        return;
    }
    int result = mosquitto_publish(mosq, NULL, topic.c_str(), payload.size(), payload.c_str(), qos, retain);
    if (result != MOSQ_ERR_SUCCESS)
    {
        std::cerr << "发布消息失败" << std::endl;
    }
}

std::string MqttThread::getTopicByIndex(int index)
{
    std::string topic = "";
    switch (index)
    {
    case MQTT_PUBLISH_TOPIC_INDEX_DATA:
        topic = mqttPublishDataTopic;
        break;
    case MQTT_PUBLISH_TOPIC_INDEX_RESPONSE:
        topic = mqttPublishResponseTopic;
        break;
    case MQTT_PUBLISH_TOPIC_INDEX_OFFLINEDATA:
        topic = mqttPublishOfflineDataTopic;
        break;
    default:
        break;
    }
    return topic;
}

void MqttThread::publish(MqttPublishUnit dataUnit, int qos, bool retain)
{
    if (!isConnected)
    {
        COUT << "Mqtt disconnected! The messenge cant't be published." << endl;
        return;
    }
    std::string mqttPublishTopic = getTopicByIndex(dataUnit.topicIndex);

    if (GlobalFlag::getInstance().bMqttDataLogOn)
    {
        COUT << "---publish--begin--" << std::endl;
        COUT << "[publish topic] : " << mqttPublishTopic << std::endl;
        COUT << dataUnit.content << std::endl;
        COUT << "---publish--end--" << std::endl;
    }
    int result = mosquitto_publish(mosq, NULL, mqttPublishTopic.c_str(), dataUnit.content.size(), dataUnit.content.c_str(), qos, retain);
    if (result != MOSQ_ERR_SUCCESS)
    {
        COUT << "发布消息失败" << std::endl;
    }
}
