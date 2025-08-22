#ifndef _Krpcprovider_H__
#define _Krpcprovider_H__

#include "google/protobuf/service.h"
#include "zookeeperutil.h"
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/InetAddress.h>
#include <google/protobuf/descriptor.h>
#include <functional>
#include <string>
#include <unordered_map>

class KrpcProvider{
    public:
        void NotifyService(google::protobuf::Service* service);
        ~KrpcProvider();
        void Run();
    private:
        muduo::net::EventLoop event_loop;
        struct ServiceInfo{
            google::protobuf::Service* service;
            std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> method_map;
        };
        std::unordered_map<std::string, ServiceInfo> service_map;
        
        void Onconnection();
        void OnMessage();
        void SendRpcResponse();
};

#endif