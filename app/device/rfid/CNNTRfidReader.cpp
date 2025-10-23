#include "CNNTRfidReader.h"
#include "Base.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sstream>
#include <iomanip>
#include "Base.h"
#include "Network.h"
#include "Utility.h"

#define DEFAULT_RFID_IP "192.168.80.111"
#define DEFAULT_RFID_PORT 6000
#define PRESET_VALUE 0xFFFF
#define POLYNOMIAL 0x8408

using namespace std;

CNNTRfidReader::CNNTRfidReader(int id)
{
}

CNNTRfidReader::~CNNTRfidReader()
{
    // Destructor implementation
}

int CNNTRfidReader::socketInit()
{
    if (connected)
    {
        COUT << "RFID server connected already!" << endl;
        return -1;
    }

    // Connect server
    COUT << "Connecting RFID server..." << ip << endl;

    clientSocket = connetServer(ip, port, TCP_BLOCK);
    if (INVALID_SOCKET == clientSocket)
    {
        COUT << "connect RFID server failed!" << ip << endl;
        return -1;
    }
    else
    {
        COUT << "Connect RFID server success!" << ip << endl;
    }
    return 1;
}

void CNNTRfidReader::socketLoop()
{
    fd_set readfds;
    struct timeval timeout;
    int bytesRead;
    char buffer[1024] = {0};
    while (!bExit)
    {
        // std::cout << "----loop---" << std::endl;
        FD_ZERO(&readfds);
        FD_SET(clientSocket, &readfds);

        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        int activity = select(clientSocket + 1, &readfds, NULL, NULL, &timeout);

        if (activity < 0)
        {
            std::cerr << "Error in select" << std::endl;
            break;
        }

        if (FD_ISSET(clientSocket, &readfds))
        {
            memset(buffer, 0, sizeof(buffer));
            bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRead < 0)
            {
                std::cerr << "Error receiving data from server" << std::endl;
                break;
            }
            else if (bytesRead == 0)
            {
                std::cout << "Server disconnected" << std::endl;
                disconnect();
                break;
            }
            else
            {
                ProcessReaderData(buffer, bytesRead);
            }
        }
    }
}

int CNNTRfidReader::socketClose()
{
    bExit = true;
    disconnect();
    return 0;
}

void CNNTRfidReader::printHex(const char *data, int length)
{
    for (int i = 0; i < length; i++)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)(unsigned char)data[i] << " ";
    }
    std::cout << std::dec << std::endl;
}

unsigned int CNNTRfidReader::uiCrc16Cal(char *pucY, unsigned char ucX)
{
    unsigned char ucI, ucJ;
    unsigned short int uiCrcValue = PRESET_VALUE;

    for (ucI = 0; ucI < ucX; ucI++)
    {
        uiCrcValue = uiCrcValue ^ *(pucY + ucI);
        for (ucJ = 0; ucJ < 8; ucJ++)
        {
            if (uiCrcValue & 0x0001)
            {
                uiCrcValue = (uiCrcValue >> 1) ^ POLYNOMIAL;
            }
            else
            {
                uiCrcValue = (uiCrcValue >> 1);
            }
        }
    }
    return uiCrcValue;
}

std::string CNNTRfidReader::hexToString(char *hexData, size_t size)
{
    std::stringstream ss;
    for (size_t i = 0; i < size; ++i)
    {
        ss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(static_cast<unsigned char>(hexData[i]));
    }
    return ss.str();
}

void CNNTRfidReader::ProcessReaderData(char *data, int dataLen)
{
    // COUT << "Received message from server: " << std::hex << data << std::endl;
    // printHex(data, dataLen);
    unsigned int caculateCrc = uiCrc16Cal(data, dataLen - 2);
    int cardNoLen = data[0];
    // COUT << "len: " << dataLen << std::endl;
    unsigned int receivedCrc = (data[dataLen - 1] << 8) | data[dataLen - 2];
    // COUT << "crc origin: " << receivedCrc << std::endl;
    // COUT << "crc: " << caculateCrc << std::endl;
    if (data[2] != 0xee)
    {
        COUT << "RFID Invalid data header" << std::endl;
    }
    if (caculateCrc == receivedCrc)
    {
        string cardNo = hexToString(data + 4, cardNoLen - 5);
        // COUT << "No: " << cardNo << std::endl;
        ProcessResult(cardNo);
    }
    else
    {
        COUT << "RFID CRC failed" << std::endl;
    }
}

void CNNTRfidReader::setSemaphore(sem_t *psem)
{
    this->semaphoreRfid = psem;
}

void CNNTRfidReader::ProcessResult(std::string strNo)
{
    // COUT << "FIN:" << side << "/RFID:" << finalCardNo << endl;
    // COUT << "CUR:" << side << "/RFID:" << strNo << endl;
    // if (pWeigh == NULL)
    //     return;
    // COUT << "WEIGH STATUS: " << pWeigh->weighStatus << endl;
}

void CNNTRfidReader::switchRFID(bool bOn)
{
    this->bSemOn = bOn;
}

bool CNNTRfidReader::initComplete()
{
    return connected;
}

// void CNNTRfidReader::saveIVRResult(PlateResult &result)
// {
//     // COUT << "license: " << result.plate << endl;
//     // COUT << "nColor: " << result.nColor << endl;
//     // COUT << "nConfidence: " << result.nConfidence << endl;
//     this->finalPlate.license = Utility::gb2312ToUtf8(result.plate);
//     this->finalPlate.nColor = result.nColor + 1;
//     this->finalPlate.nConfidence = result.nConfidence;
// }

void CNNTRfidReader::clearResult()
{
    // clear
}

// PlateResult ZKRfidReader::getPlateResult(string cardNo)
// {
//     PlateResult result;
//     result.plate = "粤A68688";
//     result.nColor = LC_BLUE;
//     result.nConfidence = 100;
//     return result;
// }

// void CNNTRfidReader::setFlag(bool *pRecognized, int *pRecognizerIndex)
// {
//     this->p_bRecognized = pRecognized;
//     this->p_recognizerIndex = pRecognizerIndex;
// }