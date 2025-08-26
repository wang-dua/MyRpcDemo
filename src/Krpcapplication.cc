#include "Krpcapplication.h"
#include <cstdlib>
#include <unistd.h>

KrpcConfig KrpcApplication::m_config;
std::mutex KrpcApplication::m_mutex;
KrpcApplication* KrpcApplication::m_application = nullptr; //单例指针
/**
 * Init 获取main中的参数, 得到配置文件路径, 加载配置文件
 * GetInstance deleteinstance 获取删除实例
 * GetConfig 配置
 */
void KrpcApplication::Init(int argc, char **argv){ 
    if (argc < 2){
        std::cout << "You need a config file: command -i <configFilePath>" << std::endl;
        exit(EXIT_FAILURE);
    }
    int o;
    std::string config_file; //文件路径
    while (-1 != (o = getopt(argc, argv, "i:"))) 
    {
        switch (o)
        {
        case 'i'://optarg是全局变量, 存储了-i后面的参数(filepath)
            config_file = optarg; 
            break;
        case '?':// 未知参数(非-i)
            std::cout << "Command: command -i <configFilePath>" << std::endl;
            exit(EXIT_FAILURE);
            break;
        case ':'://-i后没有参数
            std::cout << "Command: command -i <configFilePath>" << std::endl;
            exit(EXIT_FAILURE);
            break;
        default:
            break;
        }
    }
    m_config.LoadConfigFile(config_file.c_str()); //Krpcconfig.h中的LoadConfigFile
}
KrpcApplication &KrpcApplication::GetInstance()
{
    std::lock_guard<std::mutex> lock(m_mutex); //自动解锁
    if (m_application == nullptr){
        m_application = new KrpcApplication();
        atexit(deleteInstance); //atexit(func), 程序终止时, 调用析构函数
    }
    return *m_application;
}
void KrpcApplication::deleteInstance()
{
    if (m_application){
        delete m_application; //调用析构
    }
}
KrpcConfig &KrpcApplication::GetConfig()
{
    return m_config; //KrpcConfig m_config
}