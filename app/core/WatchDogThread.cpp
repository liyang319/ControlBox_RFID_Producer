#include "WatchDogThread.h"
#include "AppData.h"
#include <string>
#include <iostream>
#include <unistd.h>
#include "Base.h"
#include "Utility.h"

using namespace std;

WatchDogThread::WatchDogThread() : m_running(false)
{
}

WatchDogThread::~WatchDogThread()
{
    stop();
}

void WatchDogThread::start()
{
    if (!m_running)
    {
        m_running = true;
        m_thread = std::thread(&WatchDogThread::threadFunction, this);
    }
}

void WatchDogThread::stop()
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

void WatchDogThread::join()
{
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void WatchDogThread::threadFunction()
{
    while (m_running)
    {
        Utility::FeedWatchDog();
        sleep(DEFAULT_WATCHDOG_FEED_INTERVAL);
    }
}
