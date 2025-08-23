#ifndef _Krpccontroller_H
#define _Krpccontroller_H

#include <google/protobuf/service.h>
#include <string>

/**
 * KrpcController 记录和传递 RPC 调用过程中的错误、状态信息
 * 负责记录调用是否失败、失败原因，以及是否被取消。
 */

class KrpcController: public google::protobuf::RpcController{
public:
    KrpcController();
    void Reset();
    bool Failed() const;
    std::string ErrorText() const;
    void SetFailed(const std::string& reason);

    void StartCancel();
    bool IsCanceled() const;
    void NotifyOnCancel(google::protobuf::Closure* callback);
private:
    bool m_failed;
    std::string m_errText;

 };

#endif