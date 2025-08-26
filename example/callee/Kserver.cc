
#include <iostream>
#include <string>
#include "../user.pb.h"
#include "Krpcapplication.h" 
#include "Krpcprovider.h"


/*
Kserver 是服务端的主文件, 体现在UserService中, 自己写的服务类继承自user.pb.h中的UserServiceRpc类

KrpcApplication 负责全局初始化（读配置、初始化日志/网络、设置本机地址等

KrpcProvider 负责服务导出与请求分发（监听端口、注册服务、收到请求后反射式查找(服务名+方法名查找对应的虚函数)并调用目标方法
*/


/*
protobuf继承关系
google::protobuf::Service  <- UserServiceRpc (protoc生成) <- UserService (写的具体业务类)
*/
class UserService: public Kuser::UserServiceRpc
{
public:
    
    //第一个Login(也可以不同名)是服务端处理用户LoginRequest具体请求的业务逻辑
    bool Login(std::string name, std::string pwd){
        std::cout << "doing local service: Login" << std::endl;
        std::cout << "name: " << name << "   pwd: " << pwd << std::endl;
        return true;
    }


    /** 第二个Login(RpcController*, const LoginRequest*, LoginResponse*, Closure* done) 是RPC框架要求, 用于对接网络, 调用第一个Login后发送回应
     * 
     * RpcController* 是rpc控制器, 可判断超时/传递错误等信息
     * LoginRequest* 反序列化后的用户请求
     * LoginResponse* 由服务端处理的业务
     * Closure* done 
     */
    //Login 重写了基类的Login
    void Login(::google::protobuf::RpcController* controller,
              const ::Kuser::LoginRequest* request,
              ::Kuser::LoginResponse* response,
              ::google::protobuf::Closure* done){
        
                std::string name = request->name();
                std::string pwd = request->pwd();

                //
                bool login_result = Login(name, pwd); 

                Kuser::ResultCode* code = response->mutable_result();
                code->set_errcode(0);
                code->set_errmsg("");
                response->set_success(login_result);

                done->Run(); //框架向客户端发送响应
              }
};

int main(int argc, char **argv){
    KrpcApplication::Init(argc, argv); //全局框架初始化
    KrpcProvider provider;  //KrpcProvider是网络服务, 将用户服务UserService发布到rpc节点
    provider.NotifyService(new UserService());

    //启动一个rpc服务发布节点, 进程阻塞, 等待rpc的调用请求
    provider.Run();
    return 0;
}
