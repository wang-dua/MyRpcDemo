#ifndef _zookeeperutil_h_
#define _zookeeperutil_h_

#include <semaphore> //信号量
/**
 * 该文件可以讲服务信息注册到zookeeper, 客户端调用rpc时, 通过zookeeper查询可用的服务
 * ZooKeeper 官方头文件, 提供了API如
 * zookeeper_init()：连接服务器 zoo_create()：创建节点
 * zoo_get()：读取节点数据 zoo_exists()：判断节点是否存在
 */
#include <zookeeper/zookeeper.h> 
#include <string>

class ZkClient{
public:
    ZkClient();
    ~ZkClient();

    void Start(); //连接zookeeper服务器, 
    void Create(const char* path, const char* data, int datalen, int state = 0);
    std::string GetData(const char* path);
private:
    zhandle_t* m_zhandle;
};

#endif