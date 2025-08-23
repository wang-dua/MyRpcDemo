该项目使用了 protobuf glog日志库 muduo服务, zookeeper, 代码中多次出现google/protobuf/...或者google::protobuf::..., 是因为pb.h中, google 和 protobuf都是命名空间, protobuf不只是负责消息的序列化和反序列化处理, 他还自带了RPC支持框架, 包括
1. google::protobuf::Service 抽象类，表示一个“服务”，所有服务端的类都要继承它。
2. google::protobuf::MethodDescriptor 方法描述符，告诉你“这个服务里有哪些方法”。
3. google::protobuf::RpcController 控制 RPC 的错误状态、是否成功 ....    等等
muduo用于网络层, glog写日志, zookeeper用于客户端服务端互相寻找

# ./src/include, 这是全部的头文件, 包含了框架的全部方法
## 服务器:
1. Krpcapplication.h:　框架初始化　

#include "Krpcconfig.h"
#include "Krpcchannel.h" 
#include  "Krpccontroller.h"