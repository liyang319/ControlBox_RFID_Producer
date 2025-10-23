/**
 * @Class Base
 * @Brief Base基类，实现了log输出（暂未作为所有类的基类）*/

#pragma once

#include <iostream>
#include <cstdio>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <string.h>
#include <fstream>

using namespace std;
#define ENABLE_MQTT_DEBUG_INFO
#define ENABLE_GC31_DEBUG_INFO
#define ENABLE_GPS_DEBUG_INFO
#define ENABLE_UI_DEBUG_INFO
// #define ENABLE_LOGFILE
// #define ENABLE_WATCHDOG
#define DEVICE_RK2611
// #define DEVICE_RK2616

#ifndef ENABLE_LOGFILE
#define COUT (cout << "[" << Base::currentTime() << ": " << Base::fileName(new string(__FILE__)) << ": " << __func__ << ": " << __LINE__ << "]->: ")
#define CERR (cerr << "[" << Base::currentTime() << ": " << Base::fileName(new string(__FILE__)) << ": " << __func__ << ": " << __LINE__ << "]->: ")
#else
#define COUT (ofstream("weighbox_" + Base::currentDay() + ".log", ios::app) << "[" << Base::currentTime() << ": " << Base::fileName(new string(__FILE__)) << ": " << __func__ << ": " << __LINE__ << "]->: ")
#define CERR (ofstream("weighbox_" + Base::currentDay() + ".log", ios::app) << "[" << Base::currentTime() << ": " << Base::fileName(new string(__FILE__)) << ": " << __func__ << ": " << __LINE__ << "]->: ")
#endif

class Base
{
private:
    /* data */
public:
    Base(/* args */);
    ~Base();

    static string currentTime();
    static string currentDay();
    static string fileName(string *path);
};

// static std::string logFileName = "controlbox_" + Base::currentDay() + ".log";