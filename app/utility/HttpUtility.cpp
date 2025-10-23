#include "HttpUtility.h"
#include "Base.h"

size_t HttpUtility::write_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
    std::string data((const char *)ptr, (size_t)size * nmemb);
    *((std::string *)stream) += data;
    return size * nmemb;
}

size_t HttpUtility::write_callback_download(void *ptr, size_t size, size_t nmemb, void *stream)
{
    std::ofstream *file = (std::ofstream *)stream;
    file->write((const char *)ptr, size * nmemb);
    return size * nmemb;
}

CURLcode HttpUtility::httpget(const std::string &url, const std::string &params, std::string &response, long timeout)
{
    std::string full_url = url + params;
    // std::string full_url = "http://192.168.80.235:8000/otacheck?cmd=otacheck&sn=123456789&version=1.2.3";
    CURL *curl;
    CURLcode res;
    response = "";
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            if (res == CURLE_OPERATION_TIMEDOUT)
            {
                COUT << "Request timed out" << std::endl;
            }
            else
            {
                COUT << "Request failed: " << curl_easy_strerror(res) << std::endl;
            }
        }
        curl_easy_cleanup(curl);
    }
    return res;
}

CURLcode HttpUtility::httpdownload(const std::string &url, const std::string &outputFile, long timeout)
{
    if (!urlExists(url))
    {
        COUT << "URL does not exist" << std::endl;
        return CURLE_URL_MALFORMAT;
    }

    CURL *curl;
    CURLcode res;
    std::ofstream file(outputFile, std::ios::binary);

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_download);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            if (res == CURLE_OPERATION_TIMEDOUT)
            {
                COUT << "Download timed out" << std::endl;
            }
            else
            {
                COUT << "Download failed: " << curl_easy_strerror(res) << std::endl;
            }
        }
        curl_easy_cleanup(curl);
    }

    file.close();
    return res;
}

bool HttpUtility::urlExists(const std::string &url)
{
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // Perform HEAD request
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if (res == CURLE_OK)
        {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            return (response_code == 200);
        }
    }
    return false;
}

std::string HttpUtility::buildQueryString(std::map<std::string, std::string> &params)
{
    std::string queryParams = "";
    for (auto it = params.begin(); it != params.end(); ++it)
    {
        if (queryParams.empty())
        {
            queryParams += "?";
        }
        else
        {
            queryParams += "&";
        }
        queryParams += it->first + "=" + it->second;
    }
    return queryParams;
}

size_t HttpUtility::writeCallback(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

int HttpUtility::httpUploadFile(std::string url, std::string filePath, std::string fileName, long timeout)
{
    CURL *curl;
    CURLcode res;
    struct curl_httppost *formpost = NULL;
    struct curl_httppost *lastptr = NULL;
    struct curl_slist *headerlist = NULL;
    static const char buf[] = "Expect:";

    curl_global_init(CURL_GLOBAL_ALL);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "file", CURLFORM_FILE, filePath.c_str(), CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "fileName", CURLFORM_COPYCONTENTS, fileName.c_str(), CURLFORM_END);
    curl = curl_easy_init();

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_upload);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, stdout);
        headerlist = curl_slist_append(headerlist, buf);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            return 1;
        }

        curl_easy_cleanup(curl);
        curl_formfree(formpost);
        curl_slist_free_all(headerlist);
    }

    return 0;
}

size_t HttpUtility::write_callback_upload(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

int HttpUtility::httpUploadFile(std::string url, std::string filePath, std::string fileName, std::string deviceSN, long timeout)
{
    CURL *curl;
    CURLcode res;
    struct curl_httppost *formpost = NULL;
    struct curl_httppost *lastptr = NULL;
    struct curl_slist *headerlist = NULL;
    static const char buf[] = "Expect:";

    curl_global_init(CURL_GLOBAL_ALL);
    // curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "file", CURLFORM_FILE, filePath.c_str(), CURLFORM_CONTENTTYPE, "text/plain ", CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "file", CURLFORM_FILE, filePath.c_str(), CURLFORM_CONTENTTYPE, "multipart/form-data", CURLFORM_END);
    // curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "file", CURLFORM_FILE, filePath.c_str(), CURLFORM_FILENAME, fileName.c_str(), // 使用实际的文件名
    //              CURLFORM_END);
    // curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "file", CURLFORM_FILE, filePath.c_str(), CURLFORM_END);
    // curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "fileName", CURLFORM_COPYCONTENTS, fileName.c_str(), CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "deviceSN", CURLFORM_COPYCONTENTS, deviceSN.c_str(), CURLFORM_END);
    curl = curl_easy_init();

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); // 重定向
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_upload);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, stdout);
        headerlist = curl_slist_append(headerlist, buf);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            return 1;
        }
        COUT << "Upload Successfully : " << fileName << endl;
        curl_easy_cleanup(curl);
        curl_formfree(formpost);
        curl_slist_free_all(headerlist);
    }

    return 0;
}