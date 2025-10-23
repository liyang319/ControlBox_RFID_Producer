#ifndef DEVICECONFIG_H
#define DEVICECONFIG_H

#include <string>
#include <map>
#include <vector>

class DeviceConfig
{
private:
    std::map<std::string, std::map<std::string, std::string>> config;
    std::string file_path;

    DeviceConfig();
    DeviceConfig(const DeviceConfig &) = delete;
    DeviceConfig &operator=(const DeviceConfig &) = delete;

public:
    static DeviceConfig &getInstance();
    std::string get_value(const std::string &section, const std::string &key);
    void set_value(const std::string &section, const std::string &key, const std::string &value);
    void save_config();
    int get_key_count(const std::string &section);
    std::vector<std::string> get_key_list(const std::string &section);

private:
    void parse_config(const std::string &file_path);
};

#endif // DEVICECONFIG_H
