#include "../include/mprpccontroller.h"

MprpcController::MprpcController()
{
  failed_ = false;
  err_text_ = "";
}

void MprpcController::Reset()
{
  failed_ = false;
  err_text_ = "";
}
bool MprpcController::Failed() const
{
  return failed_;
}
void MprpcController::SetFailed(const std::string &reason)
{
  failed_ = true;
  err_text_ = reason;
}

std::string MprpcController::ErrorText() const
{
  return err_text_;
}

void MprpcController::StartCancel() {}
bool MprpcController::IsCanceled() const
{
  return false;
}
void MprpcController::NotifyOnCancel(google::protobuf::Closure *callback) {}