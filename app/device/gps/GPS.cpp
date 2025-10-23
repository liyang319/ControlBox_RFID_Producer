#include "GPS.h"
#include <iostream>
#include <sstream>
#include "Base.h"
#include "Utility.h"
#include "unistd.h"
#include "Device.h"
#include <vector>
#include "DataFormater.h"
#include "GlobalFlag.h"

#define MAX_TRY_CONNECT_TIME 3
#define DEFAULT_GPS_DATA_BUFFER_SIZE 512

GPS::GPS() : m_latitude(0.0), m_longitude(0.0), m_altitude(0.0)
{
    // init();
}

void GPS::init()
{
    tryCount = 0;
    while (!connected)
    {
        connectSerialPort();
        if (++tryCount > MAX_TRY_CONNECT_TIME)
            break;
        sleep(1);
    }
}

GPS::GPS(int portNumber) : m_latitude(0.0), m_longitude(0.0), m_altitude(0.0)
{
    ; // connectSerialPort(RS485_PORT);
}

void GPS::connectSerialPort()
{
    Rs485::connectSerialPort(GPS_PORT);
}

GPS::~GPS()
{
    close();
}

bool GPS::open()
{
    return isConnected();
}

void GPS::close()
{
    closeSerialPort();
}

bool GPS::readData(GPSData &gpsData)
{
    if (!isConnected())
    {
        COUT << "GPS serial not connected!!!" << endl;
        return false;
    }
    bool readSuccess = false;
    while (!readSuccess)
    {
        char buffer[DEFAULT_GPS_DATA_BUFFER_SIZE] = {0};
        int totalBytesRead = 0;
        int bytesRead = 0;

        // 读取数据直到收集到完整的数据包
        while (totalBytesRead < sizeof(buffer))
        {
            bytesRead = readDataFromPort(buffer + totalBytesRead, sizeof(buffer) - totalBytesRead);
            if (bytesRead < 0)
            {
                std::cerr << "Error reading GPS data" << std::endl;
                return false;
            }
            totalBytesRead += bytesRead;

            // 判断是否收集到完整的数据包
            if (buffer[totalBytesRead - 1] == '\n')
            {
                break;
            }
        }
        std::string gpsInfo(buffer, totalBytesRead);
        if (GlobalFlag::getInstance().bGpsDataLogOn)
        {
            COUT << "------[RecvData]--------len=" << totalBytesRead << std::endl;
            COUT << "--------RecvData--------- " << std::endl;
            COUT << gpsInfo << std::endl;
            COUT << "--------RecvData--------- " << std::endl;
        }
        // GPSData gpsData;
        //    std::string gpsInfo = "$GNRMC,062533.00,A,4003.96560,N,11620.41638,E,0.000,,081024,,,A,V*1D\n"
        //                          "$GNGGA,062533.00,4003.96560,N,11620.41638,E,1,15,1.53,152.3,M,,M,,*5C\n"
        //                          "$GNGSA,A,3,12,25,10,23,26,02,32,,,,,,2.01,1.53,1.31,1*05\n"
        //                          "$GNGSA,A,3,13,32,01,07,37,03,04,08,,,,,2.01,1.53,1.31,4*0D\n"
        //                          "$GPGSV,3,1,12,02,07,302,21,10,55,190,42,12,33,055,38,21,09,285,,0*61\n"
        //                          "$GPGSV,3,2,12,23,19,164,45,24,04,063,,25,57,103,46,26,05,201,38,0*67\n"
        //                          "$GPGSV,3,3,12,28,56,263,,32,73,355,33,194,,,44,199,,,38,0*68\n"
        //                          "$GBGSV,2,1,08,01,35,140,39,03,44,189,40,04,25,124,39,07,02,177,37,0*7D\n"
        //                          "$GBGSV,2,2,08,08,47,190,37,13,56,210,37,32,59,135,43,37,76,191,33,0*7F\n";
        readSuccess = readGPSData(gpsInfo, gpsData);

        if (GlobalFlag::getInstance().bGpsDataLogOn)
        {
            COUT << "time: " << gpsData.time << std::endl;
            COUT << "Latitude: " << gpsData.latitude << std::endl;
            COUT << "Longitude: " << gpsData.longitude << std::endl;
            COUT << "GPS Height: " << gpsData.gps_height << std::endl;
            COUT << "GPS Yaw: " << gpsData.gps_yaw << std::endl;
            COUT << "SV Num: " << gpsData.sv_num << std::endl;
            COUT << "GPS V: " << gpsData.gps_v << std::endl;
            COUT << "PDOP: " << gpsData.pdop << std::endl;
            COUT << "HDOP: " << gpsData.hdop << std::endl;
            COUT << "VDOP: " << gpsData.vdop << std::endl;
        }
    }
    return readSuccess;
}

double GPS::getLatitude()
{
    return m_latitude;
}

double GPS::getLongitude()
{
    return m_longitude;
}

double GPS::getAltitude()
{
    return m_altitude;
}

bool GPS::readGPSData(std::string data, GPSData &gpsData)
{
    std::istringstream iss(data);
    std::string line;
    if (!isValidGPSData(data))
    {
        COUT << "----invalid GPS data-----" << std::endl;
        return false;
    }
    while (std::getline(iss, line))
    {
        std::istringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;
        int index = 0;
        while (std::getline(ss, token, ','))
        {
            tokens.push_back(token);
        }
        // std::cout << "----line---" << line << "-------size=" << tokens.size() << std::endl;

        parseGPSData(tokens, gpsData);
    }
    return true;
}

#ifdef DEVICE_RK2611

bool GPS::isValidGPSData(std::string &data)
{
    bool hasRMC = false, hasGGA = false, hasGSA = false;
    std::istringstream iss(data);
    std::string line;

    while (std::getline(iss, line))
    {
        if (line.find(GPS_DATA_SECTION_GNRMC) != std::string::npos)
        {
            hasRMC = true;
        }
        else if (line.find(GPS_DATA_SECTION_GNGGA) != std::string::npos)
        {
            hasGGA = true;
        }
        else if (line.find(GPS_DATA_SECTION_GPGSA) != std::string::npos)
        {
            hasGSA = true;
        }

        if (hasRMC && hasGGA && hasGSA)
        {
            return true; // 如果同时包含$GNRMC、$GNGGA和$GNGSA数据行，则返回true
        }
    }
    return false; // 如果没有同时包含$GNRMC、$GNGGA和$GNGSA数据行，则返回false
}

void GPS::parseGPSData(std::vector<std::string> &tokens, GPSData &gpsData)
{
    if (tokens.empty())
    {
        return; // 如果tokens为空，则直接返回
    }

    if (tokens[0] == GPS_DATA_SECTION_GNRMC)
    {
        if (tokens.size() == 13)
        {
            try
            {
                gpsData.latitude = !tokens[3].empty() ? (std::stod(tokens[3].substr(0, 2)) + std::stod(tokens[3].substr(2)) / 60) : 0.0;
                gpsData.longitude = !tokens[5].empty() ? (std::stod(tokens[5].substr(0, 3)) + std::stod(tokens[5].substr(3)) / 60) : 0.0;
                gpsData.gps_v = !tokens[7].empty() ? std::stod(tokens[7]) : 0.0;
                gpsData.gps_yaw = !tokens[8].empty() ? std::stod(tokens[8]) : 0.0;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Exception $GNRMC: " << e.what() << std::endl;
            }
        }
    }
    else if (tokens[0] == GPS_DATA_SECTION_GNGGA)
    {
        if (tokens.size() == 15)
        {
            try
            {
                gpsData.latitude = !tokens[2].empty() ? (std::stod(tokens[2].substr(0, 2)) + std::stod(tokens[2].substr(2)) / 60) : 0.0;
                gpsData.longitude = !tokens[4].empty() ? (std::stod(tokens[4].substr(0, 3)) + std::stod(tokens[4].substr(3)) / 60) : 0.0;
                gpsData.gps_height = !tokens[9].empty() ? std::stod(tokens[9]) : 0.0;
                gpsData.sv_num = !tokens[7].empty() ? std::stoi(tokens[7]) : 0;
                // gpsData.gps_yaw = !tokens[8].empty() ? std::stod(tokens[8]) : 0.0; // 航向角
            }
            catch (const std::exception &e)
            {
                std::cerr << "Exception $GNGGA: " << e.what() << std::endl;
            }
        }
    }
    else if (tokens[0] == GPS_DATA_SECTION_GPGSA)
    {
        if (tokens.size() > 7)
        {
            int len = tokens.size();
            try
            {
                gpsData.pdop = !tokens[15].empty() ? std::stod(tokens[len - 3]) : 0.0;
                gpsData.hdop = !tokens[16].empty() ? std::stod(tokens[len - 2]) : 0.0;
                gpsData.vdop = !tokens[17].empty() ? std::stod(tokens[len - 1]) : 0.0;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Exception $GPGSA: " << e.what() << std::endl;
            }
        }
    }
    else if (tokens[0] == GPS_DATA_SECTION_GPGSV)
    {
        // 查找航向角信息   从单个卫星查找
    }
    gpsData.time = Base::currentTime();
}

#else

bool GPS::isValidGPSData(std::string &data)
{
    bool hasRMC = false, hasGGA = false, hasGSA = false;
    std::istringstream iss(data);
    std::string line;

    while (std::getline(iss, line))
    {
        if (line.find(GPS_DATA_SECTION_GNRMC) != std::string::npos)
        {
            hasRMC = true;
        }
        else if (line.find(GPS_DATA_SECTION_GNGGA) != std::string::npos)
        {
            hasGGA = true;
        }
        else if (line.find(GPS_DATA_SECTION_GNGSA) != std::string::npos)
        {
            hasGSA = true;
        }

        if (hasRMC && hasGGA && hasGSA)
        {
            return true; // 如果同时包含$GNRMC、$GNGGA和$GNGSA数据行，则返回true
        }
    }
    return false; // 如果没有同时包含$GNRMC、$GNGGA和$GNGSA数据行，则返回false
}

void GPS::parseGPSData(std::vector<std::string> &tokens, GPSData &gpsData)
{
    if (tokens.empty())
    {
        return; // 如果tokens为空，则直接返回
    }

    if (tokens[0] == GPS_DATA_SECTION_GNRMC)
    {
        if (tokens.size() == 14)
        {
            try
            {
                gpsData.latitude = !tokens[3].empty() ? (std::stod(tokens[3].substr(0, 2)) + std::stod(tokens[3].substr(2)) / 60) : 0.0;
                gpsData.longitude = !tokens[5].empty() ? (std::stod(tokens[5].substr(0, 3)) + std::stod(tokens[5].substr(3)) / 60) : 0.0;
                gpsData.gps_v = !tokens[7].empty() ? std::stod(tokens[7]) : 0.0;
                gpsData.gps_yaw = !tokens[8].empty() ? std::stod(tokens[8]) : 0.0;
                // 获取时间信息
                // if (!tokens[1].empty())
                // {
                //     gpsData.time = tokens[1];
                // }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Exception $GNRMC: " << e.what() << std::endl;
            }
        }
    }
    else if (tokens[0] == GPS_DATA_SECTION_GNGGA)
    {
        if (tokens.size() == 15)
        {
            try
            {
                gpsData.latitude = !tokens[2].empty() ? (std::stod(tokens[2].substr(0, 2)) + std::stod(tokens[2].substr(2)) / 60) : 0.0;
                gpsData.longitude = !tokens[4].empty() ? (std::stod(tokens[4].substr(0, 3)) + std::stod(tokens[4].substr(3)) / 60) : 0.0;
                gpsData.gps_height = !tokens[9].empty() ? std::stod(tokens[9]) : 0.0;
                gpsData.sv_num = !tokens[7].empty() ? std::stoi(tokens[7]) : 0;
                // gpsData.gps_yaw = !tokens[8].empty() ? std::stod(tokens[8]) : 0.0;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Exception $GNGGA: " << e.what() << std::endl;
            }
        }
    }
    else if (tokens[0] == GPS_DATA_SECTION_GNGSA)
    {
        if (tokens.size() == 19)
        {
            try
            {
                gpsData.pdop = !tokens[15].empty() ? std::stod(tokens[15]) : 0.0;
                gpsData.hdop = !tokens[16].empty() ? std::stod(tokens[16]) : 0.0;
                gpsData.vdop = !tokens[17].empty() ? std::stod(tokens[17]) : 0.0;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Exception $GNGSA: " << e.what() << std::endl;
            }
        }
    }
    else if (tokens[0] == GPS_DATA_SECTION_GPGSV)
    {
        // 查找航向角信息   从单个卫星查找
    }
    gpsData.time = Base::currentTime();
}
#endif
