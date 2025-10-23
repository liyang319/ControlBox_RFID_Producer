#ifndef HTTPUTILITY_H
#define HTTPUTILITY_H

#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include <map>

class HttpUtility
{
public:
    static size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream);
    static size_t write_callback_download(void *ptr, size_t size, size_t nmemb, void *stream);
    static CURLcode httpget(const std::string &url, const std::string &params, std::string &response, long timeout = 30);
    static CURLcode httpdownload(const std::string &url, const std::string &outputFile, long timeout = 30);
    static bool urlExists(const std::string &url);
    static std::string buildQueryString(std::map<std::string, std::string> &params);

    static int httpUploadFile(std::string url, std::string filePath, std::string fileName, long timeout = 30);
    static size_t writeCallback(void *ptr, size_t size, size_t nmemb, void *userdata);

    static int httpUploadFile(std::string url, std::string filePath, std::string fileName, std::string deviceSN, long timeout = 30);
    static size_t write_callback_upload(void *ptr, size_t size, size_t nmemb, void *stream);
};

#endif
