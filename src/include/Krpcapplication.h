#ifndef _Krpcapplication_H
#define _Krpcapplication_H
#include "Krpcconfig.h"
#include "Krpcchannel.h"
#include "Krpccontroller.h"
#include <mutex>

/*
KrpcApplication 负责全局初始化 读配置、初始化日志/网络、设置本机地址等
*/

class KrpcApplication
{
private:
    KrpcApplication() = default;
    ~KrpcApplication() = default;
    KrpcApplication(const KrpcApplication &) = delete;
    KrpcApplication(KrpcApplication &&) = delete;

private:
    static KrpcConfig m_config;            // 承载配置, 如监听地址、端口、注册中心/日志配置等
    static KrpcApplication *m_application; // 单例模式
    static std::mutex m_mutex;

public:
    // Init方法使用Krpcchannel, Krpccontroller 对“客户端通道”“控制器”做全局初始化
    static void Init(int argc, char **argv);
    static KrpcApplication &GetInstance(); // 使用实例
    static void deleteInstance();          // 删除实例
    static KrpcConfig &GetConfig();
};

#endif