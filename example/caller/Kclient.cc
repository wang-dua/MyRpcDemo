#include "Krpcapplication.h"
#include "../user.pb.h"
#include "Krpccontroller.h"
#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
#include "KrpcLogger.h"
#include "Krpcchannel.h"

void send_request(int thread_id, std::atomic<int>& success_count, std::atomic<int>& fail_count);

int main(int argc, char** argv){
    KrpcApplication::Init(argc, argv);

    KrpcLogger logger("MyRpcDemo");

    const int thread_count = 10; //10个线程
    const int requests_per_thread = 1;//每个线程发送一个请求

    std::vector<std::thread> threads; //
    std::atomic<int> success_count(0);
    std::atomic<int> fail_count(0);

    auto start_time = std::chrono::high_resolution_clock::now(); //计时

    //启动所有线程执行 send_request
    for (int i = 0; i < thread_count; i++){
        threads.emplace_back(
            [argc, argv, i, &success_count, &fail_count, requests_per_thread](){
                send_request(i, success_count, fail_count);
            }
        );
    }

    //阻塞主线程直到全部线程完成
    for (auto &t : threads){
        t.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    // 输出统计结果
    LOG(INFO) << "Total requests: " << thread_count * requests_per_thread;  // 总请求数
    LOG(INFO) << "Success count: " << success_count;  // 成功请求数
    LOG(INFO) << "Fail count: " << fail_count;  // 失败请求数
    LOG(INFO) << "Elapsed time: " << elapsed.count() << " seconds";  // 测试耗时
    LOG(INFO) << "QPS: " << (thread_count * requests_per_thread) / elapsed.count();//每秒请求数

    return 0;
}

//客户端调用远程服务
void send_request(int thread_id, std::atomic<int>& success_count, std::atomic<int>& fail_count){
    Kuser::UserServiceRpc_Stub stub(new KrpcChannel(false));

    Kuser::LoginRequest request;
    request.set_name("wangdua");
    request.set_pwd("password");

    Kuser::LoginResponse response;
    KrpcController controller;

    stub.Login(&controller, &request, &response, nullptr);
    if (controller.Failed()){
        std::cout << controller.ErrorText() << std::endl;
        fail_count ++;
    }
    else
    {
        if (response.result().errcode() == 0){
            std::cout << "rpc login response success: " << response.success() << std::endl;
            success_count++;
        }
        else
        {
            std::cout << "rpc login error: " << response.result().errmsg() << std::endl;
            fail_count++;
        }   
    }

}