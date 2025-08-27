#include "Krpcprovider.h"
#include "Krpcapplication.h"
#include "Krpcheader.pb.h"
#include "KrpcLogger.h"
#include <iostream>
/**
 * 注册服务对象及其方法，以便服务端能够处理客户端的RPC请求
 * google::protobuf::Service是基类, 所有服务都继承该类, 可实现多态
 */



//将服务和方法的所有信息存放到service_map
void KrpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;
    // 获取服务名字和在内的方法数量
    const google::protobuf::ServiceDescriptor *psd = service->GetDescriptor();
    std::string service_name = psd->name();
    int method_count = psd->method_count();
    KrpcLogger::Info("service name: " + service_name);

    // 将服务的每个方法存入  service_info.method_map  方法名字:方法描述符
    for (int i = 0; i < method_count; i++)
    {
        const google::protobuf::MethodDescriptor *pmd = psd->method(i);
        std::string method_name = pmd->name();
        KrpcLogger::Info("method name: " + method_name);
        service_info.method_map.emplace(method_name, pmd);
    }
    // servcie_map:  服务名字:服务信息(服务名字, 服务的方法)
    service_info.service = service;
    service_map.emplace(service_name, service_info);
}

KrpcProvider::~KrpcProvider()
{
    // std::cout << "~KrpcProvider()" << std::endl;
    KrpcLogger::Info("~KrpcProvider");
    event_loop.quit(); // 退出事件循环
}

/**
 * 配置文件通过KrpcApplication::Init()加载到config_map, 现在config_map已经有了内容
 * Run(): 使用muduo开启网络监听, 设置回复方法, 将服务和方法通过zookeeper注册
 */
void KrpcProvider::Run()
{
    //
    std::string ip = KrpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    int port = atoi(KrpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());

    muduo::net::InetAddress address(ip, port);
    std::shared_ptr<muduo::net::TcpServer> server = std::make_shared<muduo::net::TcpServer>(&event_loop, address, "KrpcProvider");
    /**
     * //连接回调//消息回调
     * 函数原型set...Callback(const ConnectionCallback& cb),
     * 参数为 std::function<void (const muduo::net::TcpConnectionPtr&)>, 是一个函数对象
     * std::bind 将函数与参数绑定为一个新的可调用函数, 也可以用lamda方法更直观
     * 使用this的原因是, 成员方法必须依赖类对象才能使用, 不能单独的作为一个函数
     * // server->setConnectionCallback(std::bind(&KrpcProvider::OnConnection, this, std::placeholders::_1));
      // server->setMessageCallback(std::bind(&KrpcProvider::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
     */
    server->setConnectionCallback(
        [this](const muduo::net::TcpConnectionPtr &con)
        {
            this->OnConnection(con);
        });
    server->setMessageCallback(
        [this](const muduo::net::TcpConnectionPtr &con, muduo::net::Buffer *buffer, muduo::Timestamp receive_time)
        {
            this->OnMessage(con, buffer, receive_time);
        });
    server->setThreadNum(4);

    /**
     * zookeeper注册服务实际上是创建一些节点, 管理的是一个类似文件系统的层级结构, 节点为znode
     * znode存储可以存储信息, 如服务的名字, 服务的方法, 可以通过路径指出
     */

    ZkClient zkclient;
    zkclient.Start();
    for (auto &sp : service_map)
    { // std::pair<const std::string, KrpcProvider::ServiceInfo>
        std::string service_path = "/" + sp.first;
        // 创建服务节点,  "/service_name", 存放数据空
        zkclient.Create(service_path.c_str(), nullptr, 0);
        for (auto &mp : sp.second.method_map)
        {
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            // 创建方法节点, "/service_name/method_name", 存放rpcserverip:rpcserverport
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            // ZOO_EPHEMERAL: 当客户端断开连接, 销毁方法节点
            zkclient.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }
    // std::cout << "RpcProvider start service at ip: " << ip << "   port: " << port << std::endl;
    KrpcLogger::Info("RpcProvider start service at ip: " + ip + "   port: " + std::to_string(port));
    server->start();
    event_loop.loop();
}



void KrpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        KrpcLogger::Info("New connection: " + conn->peerAddress().toIpPort());
    }
    else
    {
        KrpcLogger::Info("Connection closed: " + conn->peerAddress().toIpPort());
    }
}

/**
 * 处理由客户端发送的protobuf 序列化后的数据, 存放在buffer
 * 这是Reactor(事件驱动) 模式：注册回调，框架（Muduo）在事件发生时自动把正确的参数传进来, 只需要实现业务逻辑，不需要自己构造 buffer
 */
void KrpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buffer, muduo::Timestamp receive_time)
{
    // std::cout << "OnMessage" << std::endl;
    KrpcLogger::Info("OnMessage");
    // 取出buffer作为string并清空buffer
    std::string recv_buf = buffer->retrieveAllAsString();

    // coded_input 是网络的二进制字节流(未反序列化), 随着数据的取出, 指针会向后移动
    google::protobuf::io::ArrayInputStream raw_input(recv_buf.data(), recv_buf.size());
    google::protobuf::io::CodedInputStream coded_input(&raw_input);

    uint32_t header_size{};
    coded_input.ReadVarint32(&header_size); // 读取header的长度, 指针后移

    std::string rpc_header_str{};
    // Krpcheader.pb.h文件中 Krpc::RpcHeader, 创建新对象接收反序列化数据
    Krpc::RpcHeader krpcHeader;
    std::string service_name{};
    std::string method_name{};
    uint32_t args_size{};
    // 获取header, rpc_header_str
    google::protobuf::io::CodedInputStream::Limit msg_limit = coded_input.PushLimit(header_size);
    coded_input.ReadString(&rpc_header_str, header_size); // 读取header, 指针后移
    coded_input.PopLimit(msg_limit);
    // 从header读取服务 方法 参数大小
    if (krpcHeader.ParseFromString(rpc_header_str))
    {
        service_name = krpcHeader.service_name();
        method_name = krpcHeader.method_name();
        args_size = krpcHeader.args_size();
    }
    else
    {
        KrpcLogger::Error("krpcHeader parse error");
        return;
    }

    // 读取函数参数, 为什么我们知道要读取什么呢, 因为封装的顺序是自己规定的
    std::string args_str{};
    bool read_args_success = coded_input.ReadString(&args_str, args_size);
    if (!read_args_success)
    {
        KrpcLogger::Error("read args error");
        return;
    }

    /**
     * 根据消息中解析出的服务名和方法名
     * 获取具体的服务对象 和 对应的方法
     */
    auto it = service_map.find(service_name);
    if (it == service_map.end())
    {
        // std::cout << service_name << " is not exits!" << std::endl;
        KrpcLogger::Error(service_name + " is not exits!");
        return;
    }
    auto mit = it->second.method_map.find(method_name);
    if (mit == it->second.method_map.end())
    {
        // std::cout << service_name << "." << method_name << " is not exits!" << std::endl;
        KrpcLogger::Error(service_name + "." + method_name + " is not exits!");
        return;
    }
    google::protobuf::Service *service = it->second.service;
    const google::protobuf::MethodDescriptor *method = mit->second;

    // 获取客户端调用的方法(Login)的请求消息原型(LoginRequest), 反序列化, 里面有name pwd
    google::protobuf::Message* request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str))
    { // 反序列化参数
        // std::cout << service_name << "." << method_name << " parse error!" << std::endl;
        KrpcLogger::Error(service_name + "." + method_name + " parsefromstring error!");
        return;
    }

    // response由服务端填充, 并非网络字节流
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();


    //将this->KrpcProvider::SendRpcResponse()和参数conn, response 绑定为调用对象done
    google::protobuf::Closure *done = 
        google::protobuf::NewCallback<KrpcProvider, const muduo::net::TcpConnectionPtr&, google::protobuf::Message*>(this, &KrpcProvider::SendRpcResponse, conn, response);

    /**
     * 一参: 客户端调用的方法
     * 三参: 反序列化的请求, 即客户端调用方法而发送的数据
     * 四参: 空的响应对象，业务逻辑会往里填数据
     * 五参: 当done->Run(), 会执行this->SendRpcResponse()
     */
    service->CallMethod(method, nullptr, request, response, done);
    delete request;
}

void KrpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message* response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str))
    { // 序列化response 发送
        conn->send(response_str);
    }
    else
    {
        // std::cout << "serialize error!" << std::endl;
        KrpcLogger::Error("serialize error!");
    }
    delete response;
}
