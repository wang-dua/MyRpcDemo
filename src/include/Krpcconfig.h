#ifndef _Krpcconfig_H
#define _Krpcconfig_H

#include <unordered_map>
#include <string>
class KrpcConfig{
    public:
        void LoadConfigFile(const char* config_file);
        std::string Load(const std::string& key);
    private:
        std::unordered_map<std::string, std::string> config_map;
        void Trim(std::string &read_buf);
};

#endif