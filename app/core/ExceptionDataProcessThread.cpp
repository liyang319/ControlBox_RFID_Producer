#include "ExceptionDataProcessThread.h"
#include "AppData.h"
#include <string>
#include <iostream>
#include <unistd.h>
#include "AppData.h"
#include "DBWeighData.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "Base.h"
#include "WeighData.h"
#include "GlobalFlag.h"
#include "DBExceptionData.h"

using namespace std;
using namespace rapidjson;

ExceptionDataProcessThread::ExceptionDataProcessThread() : m_running(false)
{
}

ExceptionDataProcessThread::~ExceptionDataProcessThread()
{
    stop();
}

void ExceptionDataProcessThread::init()
{
    sleep(10);
}

void ExceptionDataProcessThread::start()
{
    if (!m_running)
    {
        init();
        m_running = true;
        m_thread = std::thread(&ExceptionDataProcessThread::threadFunction, this);
    }
}

void ExceptionDataProcessThread::stop()
{
    if (m_running)
    {
        m_running = false;
        if (m_thread.joinable())
        {
            m_thread.join();
        }
    }
}

void ExceptionDataProcessThread::join()
{
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void ExceptionDataProcessThread::threadFunction()
{
    while (m_running)
    {
        DBExceptionData::getInstance().deleteExpireExceptionData(10);
        sleep(10);
    }
}
