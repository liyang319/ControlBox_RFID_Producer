#ifndef GPS_H
#define GPS_H

#include <string>
#include "Rs485.h"
#include <vector>
#include "DataDef.h"

#define GPS_DATA_SECTION_GPGSA "$GPGSA"
#define GPS_DATA_SECTION_GNRMC "$GNRMC"
#define GPS_DATA_SECTION_GNGGA "$GNGGA"
#define GPS_DATA_SECTION_GNGSA "$GNGSA"
#define GPS_DATA_SECTION_GPGSV "$GPGSV"

class GPS : public Rs485
{
public:
    static GPS &getInstance()
    {
        static GPS instance;
        return instance;
    }

    void init();

    void connectSerialPort();

    bool open();
    void close();
    bool readData(GPSData &gpsData);

    double getLatitude();
    double getLongitude();
    double getAltitude();

private:
    GPS(int portNumber);
    GPS();
    ~GPS();
    GPS(const GPS &) = delete;
    GPS &operator=(const GPS &) = delete;

    double m_latitude;
    double m_longitude;
    double m_altitude;

    void parseGPSData(std::vector<std::string> &tokens, GPSData &gpsData);
    bool readGPSData(std::string data, GPSData &gpsData);
    bool isValidGPSData(std::string &data);
    int tryCount;
};

#endif
