#include "Krpcchannel.h"
#include "KrpcLogger.h"
#include "Krpcapplication.h"
#include "Krpccontroller.h"
#include "zookeeperutil.h"
#include "Krpcheader.pb.h"
#include <memory>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

std::mutex g_data_mutx;

KrpcChannel::KrpcChannel(bool connectNow) : m_clientfd(-1), m_idx(0)
{
    if (!connectNow)
        return;
    bool rt = newConnect(m_ip.c_str(), m_port);
    int count = 3;
    while (!rt && count--)
    {
        rt = newConnect(m_ip.c_str(), m_port);
    }
}

void KrpcChannel::CallMethod(const ::google::protobuf::MethodDescriptor * method, ::google::protobuf::RpcController *controller, const ::google::protobuf::Message *request, ::google::protobuf::Message *response, ::google::protobuf::Closure *done)
{
    //查找rpc服务器, 连接
    if (m_clientfd == -1){
        const google::protobuf::ServiceDescriptor* sd = method->service();
        service_name = sd->name();
        method_name = method->name();

        //启动zookeeper, 查询(RPC)服务器的ip port
        ZkClient zkClient;
        zkClient.Start();
        std::string host_data = QueryServiceHost(&zkClient, service_name, method_name, m_idx);
        m_ip = host_data.substr(0, m_idx);
        std::cout << "ip: " << m_ip << std::endl;
        m_port = atoi(host_data.substr(m_idx + 1, std::string::npos).c_str());
        std::cout << "port: " << m_port << std::endl;

        //连接服务器
        auto rt = newConnect(m_ip.c_str(), m_port);
        if (!rt){
            LOG(ERROR) << "connect server error!";
            return;
        }
        else{
            LOG(INFO) << "connect server success!";
        }
    }

    //序列化请求保存到args_str
    uint32_t args_size{};
    std::string args_str;
    if (request->SerializeToString(&args_str)){
        args_size = args_str.size();
    }
    else
    {
        controller->SetFailed("serialize request fail!");
        return;
    }

    /** 
     * 封装RPC请求的header信息, 与KrpcProvider::OnMessage(..) 相对应
    */
    Krpc::RpcHeader krpcheader;
    krpcheader.set_service_name(service_name);
    krpcheader.set_method_name(method_name);
    krpcheader.set_args_size(args_size);
    //序列化头部到rpc_header_str
    uint32_t header_size = 0;
    std::string rpc_header_str;
    if (krpcheader.SerializeToString(&rpc_header_str)){
        header_size = rpc_header_str.size();
    }
    else{
        controller->SetFailed("serialize rpc header error!");
        return;
    }

    /**
     * 依次将header_size(头部长度), rpc_header_str(头部信息, 如服务名..), 参数信息添加到send_rpc_str
     */
    std::string send_rpc_str; //rpc请求报文
    {
        //将send_rpc_str作为底层缓冲区追加
        google::protobuf::io::StringOutputStream string_output(&send_rpc_str);
        //提供对 protobuf 格式的写操作, coded_output是I/O对象
        google::protobuf::io::CodedOutputStream coded_output(&string_output);
        //将header_size和rpc_header_str 序列化后追加到send_rpc_str
        coded_output.WriteVarint32(static_cast<uint32_t>(header_size));
        coded_output.WriteString(rpc_header_str);
    }
    send_rpc_str += args_str; //header_size + rpc_header_str + args_str
    
    if (send(m_clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0) == -1){
        close(m_clientfd);
        char errtxt[512]{};
        std::cout << "send error: " << strerror_r(errno, errtxt, sizeof(errtxt)) << std::endl;
        controller->SetFailed(errtxt);
        return;
    }

    char recv_buf[1024]{};
    int recv_size = recv(m_clientfd, recv_buf, 1024, 0);
    if (recv_size == -1){
        char errtxt[512]{};
        std::cout << "recv error" << strerror_r(errno, errtxt, sizeof(errtxt)) << std::endl;
        controller->SetFailed(errtxt);
        return;
    }

    //ParseFromArray 用于从网络裸字节流解析 protobuf 消息
    if (!response->ParseFromArray(recv_buf, recv_size)){
        close(m_clientfd);
        char errtxt[512]{};
        std::cout << "parse error" << strerror_r(errno, errtxt, sizeof(errtxt)) << std::endl;
        controller->SetFailed(errtxt);
        return;
    }
    close(m_clientfd);
}

// 客户端与服务端建立连接
bool KrpcChannel::newConnect(const char *ip, uint16_t port)
{
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd = -1)
    {
        char errtxt[512] = {0};
        std::cout << "socket error" << strerror_r(errno, errtxt, sizeof(errtxt)) << std::endl;
        LOG(ERROR) << "socket error: " << errtxt;
        return false;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    int connfd = connect(clientfd, (sockaddr *)&server_addr, sizeof(server_addr));
    if (connfd = -1)
    {
        close(clientfd);
        char errtxt[512] = {0};
        std::cout << "connect error" << strerror_r(errno, errtxt, sizeof(errtxt)) << std::endl;
        LOG(ERROR) << "connect error: " << errtxt;
        return false;
    }

    m_clientfd = clientfd;
    return true;
}

//根据路径从节点获取服务的ip的port
std::string KrpcChannel::QueryServiceHost(ZkClient *zkclient, std::string service_name, std::string method_name, int &idx)
{
    std::string method_path = "/" + service_name + "/" + method_name;
    std::cout << "method_path: " << method_path << std::endl;

    
    std::unique_lock<std::mutex> lock(g_data_mutx);
    std::string host_data_1 = zkclient->GetData(method_path.c_str());
    lock.unlock();

    if (host_data_1 == ""){
        LOG(ERROR) << method_path + " is not exits!";
        return "";
    }

    idx = host_data_1.find(":");
    if (idx == std::string::npos){
        LOG(ERROR) << method_path + " address is invalid!";
    }

    return host_data_1;

}
