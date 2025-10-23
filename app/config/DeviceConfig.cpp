#include "DeviceConfig.h"
#include <fstream>
#include <iostream>
#include <algorithm>

using namespace std;

DeviceConfig::DeviceConfig() : file_path("sensor.ini")
{
    parse_config(file_path);
}

DeviceConfig &DeviceConfig::getInstance()
{
    static DeviceConfig instance;
    return instance;
}

std::string DeviceConfig::get_value(const std::string &section, const std::string &key)
{
    if (config.find(section) != config.end())
    {
        if (config[section].find(key) != config[section].end())
        {
            return config[section][key];
        }
        else
        {
            std::cerr << "Error: Key \"" << key << "\" not found in section \"" << section << "\"." << std::endl;
        }
    }
    else
    {
        std::cerr << "Error: Section \"" << section << "\" not found." << std::endl;
    }
    return "";
}

void DeviceConfig::set_value(const std::string &section, const std::string &key, const std::string &value)
{
    config[section][key] = value;
}

void DeviceConfig::save_config()
{
    std::ofstream file(file_path);
    if (!file.is_open())
    {
        std::cerr << "Error: Unable to open file for writing: " << file_path << std::endl;
        return;
    }

    for (const auto &section : config)
    {
        file << "[" << section.first << "]\n";
        for (const auto &key_value : section.second)
        {
            file << key_value.first << "=" << key_value.second << "\n";
        }
        file << "\n";
    }
    file.close();
}

void DeviceConfig::parse_config(const std::string &file_path)
{
    std::ifstream file(file_path);
    if (!file.is_open())
    {
        std::cerr << "Error: Unable to open file for reading: " << file_path << std::endl;
        return;
    }

    std::string line;
    std::string current_section;

    while (std::getline(file, line))
    {
        // 去除行末注释部分
        size_t comment_pos = line.find(';');
        if (comment_pos != std::string::npos)
        {
            line = line.substr(0, comment_pos);
        }

        // 去除多余的空格
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());

        if (!line.empty())
        {
            if (line[0] == '[' && line[line.size() - 1] == ']')
            {
                current_section = line.substr(1, line.size() - 2);
                config[current_section] = std::map<std::string, std::string>();
            }
            else
            {
                size_t delimiter_pos = line.find('=');
                if (delimiter_pos != std::string::npos && !current_section.empty())
                {
                    std::string key = line.substr(0, delimiter_pos);
                    std::string value = line.substr(delimiter_pos + 1);
                    config[current_section][key] = value;
                }
            }
        }
    }
    file.close();
}

int DeviceConfig::get_key_count(const std::string &section)
{
    if (config.find(section) != config.end())
    {
        return config[section].size();
    }
    else
    {
        std::cerr << "Error: Section \"" << section << "\" not found." << std::endl;
        return 0;
    }
}

std::vector<std::string> DeviceConfig::get_key_list(const std::string &section)
{
    std::vector<std::string> key_list;
    if (config.find(section) != config.end())
    {
        for (const auto &key_value : config[section])
        {
            key_list.push_back(key_value.first);
        }
    }
    else
    {
        std::cerr << "Error: Section \"" << section << "\" not found." << std::endl;
    }
    return key_list;
}