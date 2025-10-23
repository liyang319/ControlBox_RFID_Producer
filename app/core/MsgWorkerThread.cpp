#include "MsgWorkerThread.h"
#include "AppData.h"
#include <string>
#include <iostream>
#include <unistd.h>
#include "AppData.h"
#include "MsgDispatcher.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "Base.h"

using namespace std;
using namespace rapidjson;

MsgWorkerThread::MsgWorkerThread() : m_running(false)
{
}

MsgWorkerThread::~MsgWorkerThread()
{
    stop();
}

void MsgWorkerThread::start()
{
    if (!m_running)
    {
        m_running = true;
        m_thread = std::thread(&MsgWorkerThread::threadFunction, this);
    }
}

void MsgWorkerThread::stop()
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

void MsgWorkerThread::join()
{
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

std::vector<std::string> MsgWorkerThread::splitJsonCmd(std::string strData)
{
    std::vector<std::string> jsonDataList;
    Document document;
    document.Parse(strData.c_str());

    if (!document.IsArray())
    {
        COUT << "JSON is not an array" << std::endl;
    }
    else
    {
        for (int i = 0; i < document.Size(); i++)
        {
            StringBuffer buffer;
            Writer<StringBuffer> writer(buffer);
            document[i].Accept(writer);
            COUT << buffer.GetString() << std::endl;
            jsonDataList.push_back(buffer.GetString());
        }
    }
    return jsonDataList;
}

void MsgWorkerThread::threadFunction()
{
    while (m_running)
    {
        if (AppData::getInstance().getDataRecvQueueSize() > 0)
        {
            // 解析Mqtt进来的消息
            string recvMsg = AppData::getInstance().getDataFromDataRecvQueue();
            COUT << "[MSG:]" << recvMsg << endl;
            std::vector<std::string> cmdList = splitJsonCmd(recvMsg);
            COUT << "------cmdList----size-" << cmdList.size() << endl;

            for (std::string &cmd : cmdList)
            {
                MsgDispatcher patcher(cmd);
                patcher.dispatchMsg();
                sleep(1);
            }
        }
        usleep(10000);
        // sleep(1);
    }
}
