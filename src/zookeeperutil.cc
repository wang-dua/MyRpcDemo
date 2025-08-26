#include "zookeeperutil.h"
#include "Krpcapplication.h"
#include <mutex>
#include "KrpcLogger.h"
#include <condition_variable>

std::mutex cv_mutex;
std::condition_variable cv;
bool is_connected = false;

//事件回调函数, ZooKeeper启动线程, 调用global_watcher监听连接
void global_watcher(zhandle_t* zh, int type, int status, const char* path, void* watcherCtx){
    if (type == ZOO_SESSION_EVENT){
        if (status == ZOO_CONNECTED_STATE){
            std::lock_guard<std::mutex> lock(cv_mutex);
            is_connected = true;
        }
    }
    cv.notify_all(); //通知线程
}

ZkClient::ZkClient()
{
    m_zhandle = nullptr;
}

ZkClient::~ZkClient()
{
    if (m_zhandle != nullptr){
        zookeeper_close(m_zhandle);
    }
}

void ZkClient::Start()
{
    //直接从配置文件取值
    std::string host = KrpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = KrpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connstr = host + ":" + port;

    /**
     * 初始化zookeeper客户端, 第2参为全局 watcher，用于接收会话事件
     * 第3参为会话超时时间毫秒
     */
    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 6000, nullptr, nullptr, 0);
    if (m_zhandle == nullptr){
        KrpcLogger::Error("zookeeper_init error");
        exit(EXIT_FAILURE);
    }

    /**
     * wait检查为true, 不会阻塞直接继续
     * 当为false时, 释放unique_lock, 阻塞进程, 被唤醒后先加锁, 再次检查谓词
     */
    std::unique_lock<std::mutex> lock(cv_mutex);
    cv.wait(lock, [](){return is_connected;});
    KrpcLogger::Info("zookeeper_init success");
}

void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);

    int flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if (flag == ZNONODE){
        flag = zoo_create(m_zhandle, path, data, datalen, &ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
        if (flag == ZOK){
            KrpcLogger::Info(std::string("znode create succss...path: ") + path);
        }
        else
        {
            KrpcLogger::Error(std::string("znode create succss...path: ") + path);
            exit(EXIT_FAILURE);
        }
        

    }
}

std::string ZkClient::GetData(const char *path)
{
    char buf[64];
    int bufferlen = sizeof(buf);
    
    int flag = zoo_get(m_zhandle, path, 0, buf, &bufferlen, nullptr);
    if (flag != ZOK){
        KrpcLogger::Error("zoo_get error");
        return "";
    }
    else
    {
        return buf;
    }
    
    return "";
}
