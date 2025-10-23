#ifndef ZKRFIDREADER_H
#define ZKRFIDREADER_H

#include "TcpClient.h"
#include <semaphore.h>

class CNNTRfidReader : public TcpClient
{
public:
    CNNTRfidReader(int id);
    ~CNNTRfidReader();
    void printHex(const char *data, int length);
    // unsigned short calculateCrc16(char *data, int length);
    unsigned int uiCrc16Cal(char *pucY, unsigned char ucX);
    std::string hexToString(char *hexData, size_t size);
    void ProcessReaderData(char *data, int dataLen);

    int socketInit();
    void socketLoop();
    int socketClose();
    std::string getRfidResult() { return finalCardNo; };
    void setSemaphore(sem_t *psem);
    void ProcessResult(string strNo);
    void switchRFID(bool bOn);
    bool initComplete();
    void clearResult();
    // TH_PlateResultImage *getIVRResult();
    // void saveIVRResult(PlateResult &result);
    // PlateResult getPlateResult(string cardNo);
    // void setFlag(bool *pRecognized, int *pRecognizerIndex);

private:
    bool bExit = false;
    std::string finalCardNo;
    sem_t *semaphoreRfid;
    int readID;
    time_t lastRecognizeTime; // 上次识别结束时间
    bool bSemOn = false;
    // static sem_t semaphoreNetwork123; // 123命令线程同步
    // TH_PlateResultImage finalPlate;   // 最终识别结果
    bool *p_bRecognized;
    int *p_recognizerIndex;
    // private member variables
};

#endif // ZKRFIDREADER_H
