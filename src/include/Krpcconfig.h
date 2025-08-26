#ifndef _Krpcconfig_H
#define _Krpcconfig_H

#include <unordered_map>
#include <string>

/**
 * Krpc负责加载配置文件, 配置文件一般是key = value(rpcserver_ip=127.0.0.1 ....)
 * 配置文件存放了服务端需要监听的 rpc服务器的 IP Port等, 等待消息传入
 * 存放了客户端需要的 zookeeper的 IP Port等, zookeeper负责去哪里找服务(方法)
 * 存放了日志输出路径等
 */
class KrpcConfig
{
public:
    // LoadConfigFile 负责配置文件
    void LoadConfigFile(const char *config_file);
    std::string Load(const std::string &key); // 直接从配置文件取值
private:
    std::unordered_map<std::string, std::string> config_map; // 存储每一行的key value
    void Trim(std::string &read_buf);                        // 格式纠正
};

#endif