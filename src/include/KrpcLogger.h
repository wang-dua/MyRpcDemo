#ifndef KRPC_LOG_H
#define KRPC_LOG_H

#include <glog/logging.h>
#include <string>


class KrpcLogger{
public:
    //初始化glog日志库, 传入程序名字
    explicit KrpcLogger(const char* argv0){
        google::InitGoogleLogging(argv0);  
        FLAGS_colorlogtostderr = true; 
        FLAGS_logtostderr = true; //输出到终端
    }
    ~KrpcLogger(){
        google::ShutdownGoogleLogging();
    }
    static void Info(const std::string& message){
        LOG(INFO) << message;
    }
    static void Warning(const std::string& message){
        LOG(WARNING) << message;
    }
    static void Error(const std::string& message){
        LOG(ERROR) << message;
    }
    static void Fatal(const std::string& message){
        LOG(FATAL) << message;
    }

private:
    KrpcLogger(const KrpcLogger&) = delete;
    KrpcLogger& operator=(const KrpcLogger&) = delete;
};


#endif