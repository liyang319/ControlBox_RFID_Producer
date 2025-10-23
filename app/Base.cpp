#include <iostream>
#include <ctime>
#include <iomanip>
#include <cmath>

#include "Base.h"

using namespace std;

Base::Base(/* args */)
{
}

Base::~Base()
{
}

// 当前时间
string Base::currentTime()
{
    time_t t = time(nullptr);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %X", localtime(&t));
    return std::string(buffer);
}

string Base::currentDay()
{
    time_t t = time(nullptr);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", localtime(&t));
    return std::string(buffer);
}

string Base::fileName(string *path)
{
    string fileName = path->substr(path->find_last_of('/') + 1);
    delete path;
    return fileName;
}