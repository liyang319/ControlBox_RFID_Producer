#include "Utility.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "openssl/md5.h"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "Base.h"
#include <ctime>
#include <iconv.h>
#include "DeviceConfig.h"
#include "Version.h"
#include <fstream>
#include <string>
#include <unordered_map>
#include <algorithm>

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

std::string Utility::snCode = "";

std::string Utility::removeTrailingNewline(std::string str)
{
    std::string modifiedStr = str;
    if (!modifiedStr.empty() && modifiedStr.back() == '\n')
    {
        modifiedStr.erase(modifiedStr.length() - 1);
    }
    return modifiedStr;
}

std::string Utility::getFileContent(std::string fileName)
{
    std::ifstream file(fileName);
    std::string content = "";
    std::string line;

    if (file.is_open())
    {
        while (std::getline(file, line))
        {
            content += line;
            if (!file.eof())
            {
                content += "\n";
            }
        }
        file.close();
    }
    else
    {
        std::cerr << "Error opening file: " << fileName << std::endl;
    }

    return content;
}

int Utility::replaceFileWithCmd(std::string orginalFile, std::string newFile)
{
    std::string command = "cp " + newFile + " " + orginalFile;
    int result = system(command.c_str()); // 执行命令行

    if (result == 0)
    {
        return 0; // 成功
    }
    else
    {
        return -1; // 失败
    }
}

bool Utility::isFileEmpty(std::string filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate); // 打开文件并移动文件指针到文件末尾
    if (file.is_open())
    {
        std::streampos size = file.tellg(); // 获取文件指针位置，即文件大小
        file.close();                       // 关闭文件流

        return size == 0;
    }
    else
    {
        std::cout << "无法打开文件" << std::endl;
        return false;
    }
}

int replaceFile(const std::string orginalFile, const std::string newFile)
{
    std::ifstream newFileInput(newFile);
    if (!newFileInput)
    {
        std::cerr << "Error: Unable to open new file" << std::endl;
        return -1;
    }

    std::ofstream orginalFileOutput(orginalFile);
    if (!orginalFileOutput)
    {
        std::cerr << "Error: Unable to open orginal file" << std::endl;
        return -1;
    }

    orginalFileOutput << newFileInput.rdbuf();

    return 0;
}

int Utility::runFile(std::string executablePath, bool bBackground)
{
    // std::string command = "./" + executablePath; // 构建启动可执行文件的命令行
    // int result = system(command.c_str());        // 执行命令行
    // return result;
    std::string command = executablePath; // 使用绝对路径启动可执行文件
    if (bBackground)
        command += " &";
    int result = system(command.c_str()); // 执行命令行
    return result;
}

int Utility::killApp(std::string processName)
{
    // std::string processName = "example_process";     // 要杀死的进程名字
    std::string command = "pkill -f " + processName; // 构造要执行的命令
    int result = system(command.c_str());            // 执行命令
    if (result == 0)
    {
        COUT << "进程 " << processName << " 已成功被杀死。" << std::endl;
    }
    else
    {
        std::cerr << "无法杀死进程 " << processName << "。" << std::endl;
    }
    return result;
}

int Utility::changeFileMode(std::string filename, std::string mode)
{
    std::string command = "chmod " + mode + " " + filename;
    COUT << "---cmd = " << command << std::endl;
    int result = system(command.c_str());
    if (result == 0)
    {
        COUT << "文件权限修改成功" << std::endl;
    }
    else
    {
        std::cerr << "文件权限修改失败" << std::endl;
    }
    return result;
}

std::string Utility::calculateMD5(std::string filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return "";
    }

    MD5_CTX ctx;
    MD5_Init(&ctx);

    char buffer[1024];
    while (file)
    {
        file.read(buffer, sizeof(buffer));
        std::streamsize count = file.gcount();
        if (count > 0)
        {
            MD5_Update(&ctx, buffer, count);
        }
    }

    unsigned char md[MD5_DIGEST_LENGTH];
    MD5_Final(md, &ctx);

    std::stringstream ss;
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(md[i]);
    }

    return ss.str();
}

void Utility::fillVersionFile(std::string filename, std::string content)
{
    // Open the file in out mode to clear the original content
    std::ofstream file(filename, std::ios::out | std::ios::trunc);

    if (file.is_open())
    {
        // Write the new content to the file
        file << content;

        // Close the file
        file.close();

        COUT << "Content written to " << filename << " successfully." << std::endl;
    }
    else
    {
        std::cerr << "Error opening file " << filename << std::endl;
    }
}

int Utility::unzipFile(std::string zipFileName, std::string outputDirectory)
{
    std::string command = "unzip " + zipFileName + " -d " + outputDirectory;
    COUT << "-----unzipFile-----" << command << endl;
    int result = system(command.c_str());
    // if (result != 0)
    // {
    //     std::cerr << "Error: Failed to unzip file " << zipFileName << std::endl;
    // }
    return result;
}

std::string Utility::getFilenameFromUrl(std::string url)
{
    size_t found = url.find_last_of("/\\");
    if (found != std::string::npos)
    {
        return url.substr(found + 1);
    }
    else
    {
        return "";
    }
}

void Utility::deleteFiles(std::string dir)
{
    DIR *dp = opendir(dir.c_str());
    if (dp != NULL)
    {
        struct dirent *ep;
        while ((ep = readdir(dp)))
        {
            if (ep->d_type == DT_REG)
            { // regular file
                std::string filePath = dir + "/" + ep->d_name;
                if (remove(filePath.c_str()) != 0)
                {
                    std::cerr << "Failed to delete file: " << filePath << std::endl;
                }
            }
        }
        closedir(dp);
    }
    else
    {
        std::cerr << "Failed to open directory: " << dir << std::endl;
    }
}

bool Utility::deleteDirectory(std::string dir)
{
    DIR *dp = opendir(dir.c_str());
    if (dp != NULL)
    {
        struct dirent *ep;
        bool success = true;
        while ((ep = readdir(dp)))
        {
            if (ep->d_type == DT_DIR)
            { // directory
                if (strcmp(ep->d_name, ".") != 0 && strcmp(ep->d_name, "..") != 0)
                {
                    std::string subDirPath = dir + "/" + ep->d_name;
                    if (!deleteDirectory(subDirPath))
                    {
                        success = false;
                    }
                }
            }
            else if (ep->d_type == DT_REG)
            { // regular file
                std::string filePath = dir + "/" + ep->d_name;
                if (remove(filePath.c_str()) != 0)
                {
                    std::cerr << "Failed to delete file: " << filePath << std::endl;
                    success = false;
                }
            }
        }
        closedir(dp);
        if (rmdir(dir.c_str()) != 0)
        {
            std::cerr << "Failed to delete directory: " << dir << std::endl;
            success = false;
        }
        return success;
    }
    else
    {
        std::cerr << "Failed to open directory: " << dir << std::endl;
        return false;
    }
}

bool Utility::createDirIfNotExist(std::string dirPath)
{
    struct stat st;
    if (stat(dirPath.c_str(), &st) != 0)
    {
        if (mkdir(dirPath.c_str(), 0777) == 0)
        {
            return true;
        }
        else
        {
            std::cerr << "Failed to create directory: " << dirPath << std::endl;
            return false;
        }
    }
    else
    {
        if (S_ISDIR(st.st_mode))
        {
            COUT << "Directory already exists: " << dirPath << std::endl;
            return true;
        }
        else
        {
            std::cerr << dirPath << " is not a directory" << std::endl;
            return false;
        }
    }
}

bool Utility::copyFileTo(std::string filePath, std::string dstPath)
{
    std::ifstream srcFile(filePath, std::ios::binary);
    if (!srcFile)
    {
        std::cerr << "Failed to open source file: " << filePath << std::endl;
        return false;
    }

    std::string dstFilePath = dstPath + "/" + filePath.substr(filePath.find_last_of('/') + 1);
    std::ofstream dstFile(dstFilePath, std::ios::binary);
    if (!dstFile)
    {
        srcFile.close();
        std::cerr << "Failed to create destination file: " << dstFilePath << std::endl;
        return false;
    }
    dstFile << srcFile.rdbuf();

    srcFile.close();
    dstFile.close();

    return true;
}

bool Utility::fileExists(std::string fileName)
{
    std::ifstream file(fileName);
    return file.good();
}

bool Utility::isExecutable(std::string fileName)
{
    return access(fileName.c_str(), X_OK) == 0;
}

bool Utility::startApp(std::string executablePath, bool bBackground)
{
    if (!fileExists(executablePath))
    {
        COUT << "File " << executablePath << " does not exist" << std::endl;
        return false;
    }
    if (!isExecutable(executablePath))
    {
        COUT << "File " << executablePath << " is not executable" << std::endl;
        return false;
    }
    std::string command = executablePath; // 使用绝对路径启动可执行文件
    if (bBackground)
        command += " &";
    int result = system(command.c_str());
    if (result == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Utility::FeedWatchDog()
{
    std::cout << "-----------FeedWatchDog----------" << endl;
    system("echo 1 > /dev/watchdog");
}

void Utility::CloseWatchDog()
{
    std::cout << "-----------CloseWatchDog----------" << endl;
    system("echo V > /dev/watchdog");
}

std::string Utility::hexToString(unsigned char *data, int dataLen)
{
    std::stringstream ss;
    for (size_t i = 0; i < dataLen; ++i)
    {
        ss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(static_cast<unsigned char>(data[i]));
    }
    return ss.str();
}

std::string Utility::hexToString2(unsigned char *data, int dataLen)
{
    std::ostringstream oss;

    // 每两个字节进行调换
    for (int i = 0; i < dataLen; i += 2)
    {
        if (i + 1 < dataLen)
        {
            // 输出第二个字节
            oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(data[i + 1]);
            // 输出第一个字节
            oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
        }
        else
        {
            // 如果是奇数个字节，直接输出最后一个字节
            oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
        }
    }

    return oss.str();
}

std::string Utility::getSystemVersion()
{
    std::ifstream os_release("/etc/os-release");
    std::string line;
    std::string version_id = "";
    while (std::getline(os_release, line))
    {
        if (line.find("VERSION_ID=") != std::string::npos)
        {
            std::istringstream iss(line.substr(line.find("=") + 1));
            iss >> version_id;
            // Remove the quotes from the version_id
            version_id = version_id.substr(1, version_id.size() - 2);
            break;
        }
    }
    os_release.close();
    return version_id;
}

bool Utility::parseDeviceSN1804(std::string &strSN)
{
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    strSN = "";
    while (std::getline(cpuinfo, line))
    {
        if (line.find("Serial") != std::string::npos)
        {
            strSN = line.substr(line.find(":") + 2);
            break;
        }
    }
    if (strSN.empty())
    {
        COUT << "getDeviceSN failed!" << endl;
        return false;
    }
    cpuinfo.close();
    return true;
}

// bool Utility::parseDeviceSN2004(std::string &strSN)
// {
//     std::ifstream sys_info("/sys/class/sunxi_info/sys_info");
//     if (!sys_info.is_open())
//     {
//         std::cerr << "Error: Unable to open sys_info file" << std::endl;
//         return false;
//     }

//     std::string line;
//     while (std::getline(sys_info, line))
//     {
//         if (line.find("sunxi_chipid") != std::string::npos)
//         {
//             size_t pos = line.find(":");
//             if (pos != std::string::npos)
//             {
//                 strSN = line.substr(pos + 2);
//                 break;
//             }
//         }
//     }
//     sys_info.close();
//     return true;
// }
bool Utility::parseDeviceSN2004(std::string &strSN)
{
    std::ifstream file("/sys/devices/platform/rkid/rksnid");

    // 检查文件是否成功打开
    if (!file.is_open())
    {
        return false; // 文件打开失败
    }
    // 读取文件内容
    std::getline(file, strSN);
    // 关闭文件
    file.close();

    // 检查读取的内容是否为空
    return !strSN.empty();
}

bool Utility::initDevice()
{
    bool retVal = false;
    string strVer = getSystemVersion();
    if (strVer == "18.04")
    {
        retVal = parseDeviceSN1804(snCode);
    }
    else if (strVer == "20.04")
    {
        retVal = parseDeviceSN2004(snCode);
    }
    cout << "[Device SN] : " << snCode << endl;
    saveSN();
    saveVersion();
    return retVal;
}

std::string Utility::getDeviceSN()
{
    // return snCode;
    return "011R611065665436";
}

time_t Utility::getTimestamp()
{
    return time(nullptr);
}

Value Utility::getStringValue(const std::string &str, Document::AllocatorType &allocator)
{
    return Value(str.c_str(), allocator);
}

void Utility::convert_endian(uint8_t *buf, int len)
{
    for (int i = 0; i < len / 2; i++)
    {
        std::swap(buf[i], buf[len - i - 1]);
    }
}

std::string Utility::convertToGB2312(std::string &input)
{
    iconv_t cd = iconv_open("GB2312", "UTF-8");
    if (cd == reinterpret_cast<iconv_t>(-1))
    {
        std::cerr << "Error initializing iconv: " << strerror(errno) << std::endl;
        return "";
    }

    size_t inbytesleft = input.size();
    size_t outbytesleft = inbytesleft * 4; // GB2312 may need more space than UTF-8
    char *inbuf = const_cast<char *>(input.c_str());
    char *outbuf = new char[outbytesleft];
    char *outptr = outbuf;

    memset(outbuf, 0, outbytesleft);
    size_t result = iconv(cd, &inbuf, &inbytesleft, &outptr, &outbytesleft);
    if (result == static_cast<size_t>(-1))
    {
        std::cerr << "Error during iconv: " << strerror(errno) << std::endl;
        delete[] outbuf;
        iconv_close(cd);
        return "";
    }

    std::string output(outbuf, outptr);
    delete[] outbuf;
    iconv_close(cd);
    return output;
}

std::string Utility::convertGB2312ToUTF8(std::string &input)
{
    iconv_t cd = iconv_open("UTF-8", "GB2312");
    if (cd == reinterpret_cast<iconv_t>(-1))
    {
        std::cerr << "Error initializing iconv: " << strerror(errno) << std::endl;
        return "";
    }

    size_t inbytesleft = input.size();
    size_t outbytesleft = inbytesleft * 4; // UTF-8 may need more space than GB2312
    std::vector<char> outbuf(outbytesleft);
    char *inbuf = const_cast<char *>(input.c_str());
    char *outptr = outbuf.data();

    memset(outbuf.data(), 0, outbytesleft);

    size_t result = iconv(cd, &inbuf, &inbytesleft, &outptr, &outbytesleft);
    if (result == static_cast<size_t>(-1))
    {
        std::cerr << "Error during iconv: " << strerror(errno) << std::endl;
        iconv_close(cd);
        return "";
    }

    // Calculate the number of bytes written to the output buffer
    size_t output_size = outbytesleft;                   // Remaining bytes in outbuf
    size_t converted_size = outbuf.size() - output_size; // Total bytes written
    std::string output(outbuf.data(), converted_size);

    iconv_close(cd);
    return output;
}

void Utility::rebootSystem()
{
    int result = system("reboot");
    if (result == 0)
    {
        std::cout << "System rebooting..." << std::endl;
    }
    else
    {
        std::cout << "Failed to reboot system." << std::endl;
    }
}

void Utility::saveSN()
{
    // 输出SN到文件
    string snFile = DeviceConfig::getInstance().get_value("dtu", "SNFile");
    std::ofstream file(snFile);

    string sn = Utility::snCode;
    COUT << "SN: " << sn << endl;

    if (!file.is_open())
    {
        CERR << "无法打开SN文件" << endl;
    }
    else
    {
        // 写到 /var/sn 文件中，工作人员读取SN，并进行条码打印
        file << sn << endl;
        file.close();
    }
}

void Utility::saveVersion()
{
    // 输出版本号到文件
    string versionFile = DeviceConfig::getInstance().get_value("dtu", "VersionFile");
    std::ofstream vfile(versionFile);
    COUT << "Version: " << VERSION << endl;

    if (!vfile.is_open())
    {
        CERR << "无法打开Version文件" << endl;
    }
    else
    {
        // 写到 /var/version 文件中
        vfile << VERSION << endl;
        vfile.close();
    }
}

bool Utility::saveTareWeight(std::string filePath, float tareWeight)
{
    std::ofstream file(filePath);
    if (!file.is_open())
    {
        file.open(filePath, std::ios::out);
    }

    if (file.is_open())
    {
        file << "TareWeight=" << tareWeight << std::endl;
        file.close();
        return true;
    }
    else
    {
        return false;
    }
}

bool Utility::readTareWeight(std::string filePath, float &tareWeight)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        return false;
    }

    std::string line;
    if (std::getline(file, line))
    {
        size_t found = line.find("=");
        if (found != std::string::npos)
        {
            tareWeight = std::stof(line.substr(found + 1));
            file.close();
            return true;
        }
        else
        {
            file.close();
            return false;
        }
    }
    else
    {
        file.close();
        return false;
    }
}

bool Utility::readFileContent(std::string filePath, std::string &fileContent)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    fileContent = buffer.str();

    return true;
}

std::string Utility::getTimeFromTimestamp(int timestamp)
{
    // 将时间戳转换为 time_t 类型
    std::time_t time = static_cast<std::time_t>(timestamp);

    // 将 time_t 转换为 tm 结构
    std::tm *tmPtr = std::localtime(&time);

    // 使用 stringstream 格式化输出
    std::ostringstream oss;
    oss << std::put_time(tmPtr, "%Y-%m-%d %H:%M:%S");

    return oss.str();
}

bool Utility::readMyConfig(std::string filePath, std::string key, std::string &value)
{
    std::ifstream configFile(filePath);
    if (!configFile.is_open())
    {
        return false; // 文件不存在或打开失败
    }

    std::string line;
    while (std::getline(configFile, line))
    {
        std::istringstream iss(line);
        std::string k, v;
        if (std::getline(iss, k, '=') && std::getline(iss, v))
        {
            if (k == key)
            {
                value = v;   // 直接赋值为字符串
                return true; // 找到键并成功读取值
            }
        }
    }

    return false; // 没有找到该键
}

bool Utility::writeMyConfig(std::string filePath, std::string key, std::string value)
{
    std::unordered_map<std::string, std::string> configMap;
    std::ifstream configFile(filePath);

    // 如果文件存在，读取现有的键值对
    if (configFile.is_open())
    {
        std::string line;
        while (std::getline(configFile, line))
        {
            std::istringstream iss(line);
            std::string k, v;
            if (std::getline(iss, k, '=') && std::getline(iss, v))
            {
                configMap[k] = v; // 读取键值对
            }
        }
        configFile.close();
    }

    // 修改或添加新的键值对
    configMap[key] = value;

    // 写回文件
    std::ofstream outFile(filePath);
    if (!outFile.is_open())
    {
        return false; // 打开文件失败
    }

    for (const auto &pair : configMap)
    {
        outFile << pair.first << "=" << pair.second << "\n";
    }

    outFile.close();
    return true; // 成功写入
}

bool Utility::stringToBool(std::string str)
{
    return str != "0"; // 如果 str 不是 "0"，则返回 true；否则返回 false
}

std::string Utility::getFileExtension(std::string &filePath)
{
    // 查找最后一个 '.' 的位置
    size_t dotPos = filePath.find_last_of('.');
    // 查找最后一个 '/' 的位置
    size_t slashPos = filePath.find_last_of("/\\");
    // 确保 '.' 在最后一个 '/' 之后
    if (dotPos != std::string::npos && (slashPos == std::string::npos || dotPos > slashPos))
    {
        return filePath.substr(dotPos + 1); // 返回扩展名，不包括 '.'
    }
    return ""; // 没有扩展名
}

bool Utility::copyFile(std::string &srcFile, std::string &destFile)
{
    // 打开源文件
    std::ifstream src(srcFile, std::ios::binary);
    if (!src)
    {
        std::cerr << "Error opening source file: " << srcFile << " - " << std::strerror(errno) << std::endl;
        return false;
    }

    // 打开目标文件
    std::ofstream dest(destFile, std::ios::binary);
    if (!dest)
    {
        std::cerr << "Error opening destination file: " << destFile << " - " << std::strerror(errno) << std::endl;
        return false;
    }

    // 复制文件内容
    dest << src.rdbuf();

    // 检查是否复制成功
    if (src.bad() || dest.bad())
    {
        std::cerr << "Error during file copy." << std::endl;
        return false;
    }

    return true;
}

bool Utility::removeFile(std::string &filePath)
{
    struct stat buffer;
    // 检查文件是否存在
    if (stat(filePath.c_str(), &buffer) == 0)
    {
        // 尝试删除文件
        if (remove(filePath.c_str()) == 0)
        {
            return true; // 删除成功
        }
        else
        {
            std::cerr << "Error deleting file: " << strerror(errno) << std::endl;
            return false; // 删除失败
        }
    }
    else
    {
        std::cerr << "File does not exist: " << filePath << std::endl;
        return false; // 文件不存在
    }
}

bool Utility::isDigits(const std::string &str)
{
    for (char c : str)
    {
        if (!isdigit(c))
        {
            return false; // 如果有非数字字符，返回 false
        }
    }
    return true; // 全部字符都是数字
}

// 将字符串转换为小写
std::string toLower(const std::string &str)
{
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

bool Utility::isProcessRunning(const std::string &processName)
{
    DIR *dir = opendir("/proc");
    struct dirent *entry;

    if (!dir)
        return false;

    while ((entry = readdir(dir)) != nullptr)
    {
        // 只处理目录，且目录名是数字（PID）
        if (entry->d_type == DT_DIR && isDigits(entry->d_name))
        {
            std::string pid = entry->d_name;
            std::ifstream statusFile("/proc/" + pid + "/comm");
            std::string name;
            if (statusFile)
            {
                std::getline(statusFile, name);
                // 去除读取的进程名的空白字符
                name.erase(remove_if(name.begin(), name.end(), ::isspace), name.end());
                // 检查进程名是否匹配（不区分大小写）
                if (toLower(name) == toLower(processName))
                {
                    closedir(dir);
                    return true; // 找到同名进程
                }
            }
        }
    }
    closedir(dir);
    return false; // 未找到同名进程
}