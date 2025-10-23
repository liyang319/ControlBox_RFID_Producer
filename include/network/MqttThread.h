#ifndef MQTT_THREAD_H
#define MQTT_THREAD_H

#include <iostream>
#include <thread>
#include <mosquitto.h>
#include "MqttReportThread.h"
#include "DataDef.h"

// class MqttReportThread;
// MQTT服务器地址和端口号
// #define MQTT_HOST "192.168.80.25"
// #define MQTT_PORT 1883

// // MQTT客户端ID
// #define MQTT_CLIENT_ID "mqtt_cpp_client"

// // MQTT主题
// #define MQTT_SUBSCRIBE_TOPIC "test_topic"
// #define MQTT_PUBLISH_TOPIC "test_topic_pub"

// 消息内容
#define MQTT_CONNECT_TIMEOUT 5

class MqttThread
{
public:
    MqttThread();
    ~MqttThread();
    void init();
    void start();
    void stop();
    void join();
    void publish(const std::string &topic, const std::string &payload, int qos, bool retain);
    void publish(MqttPublishUnit dataUnit, int qos, bool retain); // 不传topic，默认走
    bool isMqttConnected() { return isConnected; }
    std::string getTopicByIndex(int index);

private:
    static bool isConnected;
    struct mosquitto *mosq;
    std::thread thread;
    static bool isRunning;
    std::string mqttHost;
    std::string mqttPort;
    std::string mqttUsername;
    std::string mqttPassword;
    std::string mqttClinetID;
    std::string mqttPublishDataTopic;
    std::string mqttPublishResponseTopic;
    std::string mqttPublishOfflineDataTopic;
    static std::string mqttSubscribeTopic;

    MqttReportThread reportThread; // DTU上报线程

    static void message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message);
    static void connect_callback(struct mosquitto *mosq, void *userdata, int result);
    static void disconnect_callback(struct mosquitto *mosq, void *userdata, int result);
};

#endif // MQTT_THREAD_H
