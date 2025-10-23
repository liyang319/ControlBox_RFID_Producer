#ifndef UTILITY_H
#define UTILITY_H

#include <string>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;
class Utility
{
public:
    static std::string getDeviceSN();
    static std::string removeTrailingNewline(std::string str);
    static std::string getFileContent(std::string filename);
    // static std::string calculateMD5(const std::string &file_path);
    static int replaceFileWithCmd(std::string orginalFile, std::string newFile);
    static int replaceFile(const std::string orginalFile, const std::string newFile);
    static int runFile(std::string executablePath, bool bBackground);
    static int killApp(std::string processName);
    static int changeFileMode(std::string filename, std::string mode);
    static std::string calculateMD5(std::string filename);
    static void fillVersionFile(std::string filename, std::string content);
    static int unzipFile(std::string zipFileName, std::string outputDirectory);
    static std::string getFilenameFromUrl(std::string url);
    static void deleteFiles(std::string dir);
    static bool deleteDirectory(std::string dir);
    static bool createDirIfNotExist(std::string dirPath);
    static bool copyFileTo(std::string filePath, std::string dstPath);
    static bool fileExists(std::string fileName);
    static bool isExecutable(std::string fileName);
    static bool startApp(std::string executablePath, bool bBackground);
    static bool isFileEmpty(std::string filename);
    static void FeedWatchDog();
    static void CloseWatchDog();
    static std::string hexToString(unsigned char *data, int dataLen);
    static std::string hexToString2(unsigned char *data, int dataLen);

    static time_t getTimestamp();
    static Value getStringValue(const std::string &str, Document::AllocatorType &allocator);
    static std::string convertToGB2312(std::string &input);
    static std::string convertGB2312ToUTF8(std::string &input);
    static void convert_endian(uint8_t *buf, int len);
    static bool initDevice();
    static std::string getSystemVersion();
    // static bool parseDeviceSN1804(string &strSN);
    static bool parseDeviceSN1804(std::string &strSN);
    static bool parseDeviceSN2004(std::string &strSN);
    static std::string snCode;
    static void rebootSystem();
    static void saveSN();
    static void saveVersion();
    static bool saveTareWeight(std::string filePath, float tareWeight);
    static bool readTareWeight(std::string filePath, float &tareWeight);
    static bool readFileContent(std::string filePath, std::string &fileContent);
    static std::string getTimeFromTimestamp(int timestamp);

    static bool readMyConfig(std::string filePath, std::string key, std::string &value);
    static bool writeMyConfig(std::string filePath, std::string key, std::string value);
    static bool stringToBool(std::string str);
    static std::string getFileExtension(std::string &filePath);
    static bool copyFile(std::string &srcFile, std::string &destFile);
    static bool removeFile(std::string &filePath);

    static bool isDigits(const std::string &str);
    static bool isProcessRunning(const std::string &processName);
};

#endif // UTILITY_H
