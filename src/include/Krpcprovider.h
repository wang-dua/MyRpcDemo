#ifndef _Krpcprovider_H__
#define _Krpcprovider_H__

//service.h中包含基类Seevice
#include "google/protobuf/service.h"
//描述符, 服务描述符/方法描述符
#include <google/protobuf/descriptor.h> 
//Zookeeper 交互的工具类
#include "zookeeperutil.h"
// muduo在RPC中做网络层使用, 可以处理大量并发请求
#include <muduo/net/TcpServer.h> 
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/InetAddress.h>
#include <functional>
#include <string>
#include <unordered_map>

/**
 * RPC服务端的核心代码: 服务发布中心
 */

class KrpcProvider{
public:
    //在RPC中注册服务UserService
    void NotifyService(google::protobuf::Service* service); 
    ~KrpcProvider();

    //启动RPC服务: 启动 muduo 服务器，进入事件循环，等待客户端请求
    void Run(); 
private:
    /**
     * event_loop 循环监听事件: 新客户端连接, 客户端发消息, 可以发送消息...
     * muduo 的事件循环就是基于 I/O多路复用中的 epoll
     * muduo将epoll封装, 处理事件只需要写回调函数 Onconnection, OnMessage....
     */
    muduo::net::EventLoop event_loop; 

    //服务的信息
    struct ServiceInfo{
        //具体的服务对象
        google::protobuf::Service* service;
        //MethodDescriptor是方法描述符, method_map 可以根据方法名(字符串) 查找方法描述符
        //描述符里有 方法名, 参数类型, 返回类型,   以调用该方法
        //方法名->描述符
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> method_map;
    };
    //所有注册的服务信息, 服务名->服务信息
    std::unordered_map<std::string, ServiceInfo> service_map;
    
    //客户端连接(断开), 收到客户端消息, 向客户端发送RPC回应 的处理
    void Onconnection();
    void OnMessage();
    void SendRpcResponse();
};

#endif