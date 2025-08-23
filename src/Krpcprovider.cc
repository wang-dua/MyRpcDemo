#include "Krpcprovider.h"
#include "Krpcapplication.h"
#include "Krpcheader.pb.h"
#include "KrpcLogger.h"
#include <iostream>
/**
 * 注册服务对象及其方法，以便服务端能够处理客户端的RPC请求
 * google::protobuf::Service是基类, 所有服务都继承该类, 可实现多态
 */
void KrpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;
    //获取服务名字和在内的方法数量
    const google::protobuf::ServiceDescriptor *psd = service->GetDescriptor();
    std::string service_name = psd->name();
    int method_count = psd->method_count();
    std::cout << "service_name: " << service_name << std::endl;

    //将服务的每个方法存入  service_info.method_map  方法名字:方法描述符
    for (int i = 0; i < method_count; i++){
        const google::protobuf::MethodDescriptor* pmd = psd->method(i);
        std::string method_name = pmd->name();
        std::cout << "method_name = " << method_name << std::endl;
        service_info.method_map.emplace(method_name, pmd);
    }
    //servcie_map:  服务名字:服务信息(服务名字, 服务的方法)
    service_info.service = service;
    service_map.emplace(service_name, service_info);
}
/**
 * 配置文件已经通过KrpcApplication::Init()加载到config_map
 */
void KrpcProvider::Run()
{   
    //
    std::string ip = KrpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    int port = atoi(KrpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());

    muduo::net::InetAddress address(ip, port);
    std::shared_ptr<muduo::net::TcpServer> server = std::make_shared<muduo::net::TcpServer>(&event_loop, address, "KrpcProvider");
}

void KrpcProvider::Onconnection()
{

}

void KrpcProvider::OnMessage()
{
}

void KrpcProvider::SendRpcResponse()
{
}
