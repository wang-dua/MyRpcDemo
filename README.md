该项目使用了 protobuf glog日志库 muduo服务, zookeeper,  muduo用于网络层, glog写日志, zookeeper用于客户端服务端互相寻找, 代码中多次出现google/protobuf/...或者google::protobuf::..., 是因为pb.h中, google 和 protobuf都是命名空间, protobuf不只是负责消息的序列化和反序列化处理, 他还自带了RPC支持框架, 包括
1. google::protobuf::Service 抽象类，表示一个“服务”，所有服务端的类都要继承它。
2. google::protobuf::MethodDescriptor 方法描述符，告诉你“这个服务里有哪些方法”。
3. google::protobuf::RpcController 控制 RPC 的错误状态、是否成功 ....    等等

代码中一直反复说的服务注册是指**把某个服务的信息登记到一个“服务管理中心”，让别人能够发现并调用它**。

# ./src/include, 这是全部的头文件, 包含了框架的全部方法
## 服务器:
1. **Krpcapplication.h:**　

   框架初始化　

   #include "Krpcconfig.h"
   #include "Krpcchannel.h" 
   #include  "Krpccontroller.h"

2. **Krpcconfig.h:**

   加载配置文件, 配置文件中包含了 RPC , zookeeper等需要的IP Port等

3. **Krpcprovider.h**:

   \#include "google/protobuf/service.h"

   \#include<google/protobuf/descriptor.h>

   \#include "zookeeperutil.h"

   \#include<muduo/net/......>

   发布服务

4. **zookeeperutil.h**:

   RPC 框架的 **服务注册中心客户端接口**

5. **Krpccontroller：** 

   记录和传递 RPC 调用过程中的错误、状态信息

6. **Krpclogger**: 使用glog记录日志

7. **Krpcheader.pb.h:** 
