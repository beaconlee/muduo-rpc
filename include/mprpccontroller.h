#ifndef __MP__RPC__CONTROLLER__H__
#define __MP__RPC__CONTROLLER__H__

#include <google/protobuf/service.h>
#include <string>

class MprpcController : public google::protobuf::RpcController
{
public:
  MprpcController();

  void Reset();
  bool Failed() const;
  void SetFailed(const std::string &reason);

  std::string ErrorText() const;

  // 目前尚未实现的功能
  void StartCancel();
  bool IsCanceled() const;
  void NotifyOnCancel(google::protobuf::Closure *callback);

private:
  bool        failed_;
  std::string err_text_;
};


#endif //__MP__RPC__CONTROLLER__H__