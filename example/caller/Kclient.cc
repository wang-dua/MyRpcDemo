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

int main(){

    return 0;
}

//客户端调用远程服务
void send_request(int thread_id, std::atomic<int>& success_count, std::atomic<int>& fail_count){
    Kuser::UserServiceRpc_Stub stub(new KrpcChannel(false));
}
