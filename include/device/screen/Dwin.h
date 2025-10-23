#ifndef DWIN_H
#define DWIN_H

#include <string>
#include "Rs485.h"
#include <vector>
#include "DataDef.h"

class Dwin : public Rs485
{
public:
    static Dwin &getInstance()
    {
        static Dwin instance;
        return instance;
    }

    void init();

    void connectSerialPort(int portNumber, int baudrate, int databit, int stopbit, int parity);
    void connectSerialPort();

    int readData(uint8_t *buf, int len);

    bool open();
    void close();

    bool getVersion();
    bool readFloatValue(uint16_t addr, float &result);
    bool writeFloatValue(uint16_t addr, float data);

    // bool readStringValue(u_int16_t addr, std::string &result);
    bool clearStringValue(uint16_t addr, int len);
    bool writeStringValue(uint16_t addr, std::string &result);
    bool readStringValue(uint16_t addr, std::string &result, int len);
    bool syncSystemTime();
    bool setIcon(uint16_t addr, uint16_t index);
    bool switchPage(int pageIndex);
    bool getDwinVersion(string &strVersion);
    string strSN;

    bool doDwinOTA(string fileName);
    bool downloadFile(std::string filePath);
    bool resetDwin();

private:
    bool initialized;
    Dwin(int portNumber);
    Dwin();
    ~Dwin();
    Dwin(const Dwin &) = delete;
    Dwin &operator=(const Dwin &) = delete;
    int tryCount;
};

#endif
