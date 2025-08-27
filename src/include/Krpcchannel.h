#ifndef _Krpcchannel_H_
#define _Krpcchannel_H_

#include <google/protobuf/service.h>
#include "zookeeperutil.h"

/**
 * 将客户端发出的 RPC 调用请求发送到远程服务器，以及管理与服务器的连接
 */

class KrpcChannel : public google::protobuf::RpcChannel
{
public:
    KrpcChannel(bool connectNow); // 构造 KrpcChannel 对象, 是否创建对象就立即建立TCP连接
    virtual ~KrpcChannel() {}
    /**
     * CallMethod属于RpcChannel::CallMethod 在客户端调用，用于把请求发送给远程服务器
     * 这里的CallMethod与KrpcProvider中的service->CallMethod不同, 后者用于执行服务
     */
    void CallMethod(const ::google::protobuf::MethodDescriptor *method, ::google::protobuf::RpcController* controller, const ::google::protobuf::Message* request, ::google::protobuf::Message* response, ::google::protobuf::Closure* done) override;
private:
    int m_clientfd;
    std::string service_name{};
    std::string m_ip{};
    uint16_t m_port{};
    std::string method_name{};
    int m_idx{}; // m_idx为":"下标, 划分服务器ip和port的下标, 便于取值
    bool newConnect(const char* ip, uint16_t port); //建立 TCP 连接到远程服务器
    std::string QueryServiceHost(ZkClient* zkclient, std::string  service_name, std::string method_name, int &idx); //向 Zookeeper 查询远程服务的IP和port

};

#endif