#ifndef __MP__RPC__APPLICATION__H__
#define __MP__RPC__APPLICATION__H__

// mprpc 框架的初始化类

#include "mprpcconfig.h"

class MprpcApplication
{
public:
  static void Init(int argc, char **argv);
  static MprpcApplication &GetInstance();
  static MprpcConfig &GetConfig();

private:
  static MprpcConfig  config_;

  MprpcApplication();
  MprpcApplication(const MprpcApplication &) = delete;
  MprpcApplication(MprpcApplication &&) = delete;
  ~MprpcApplication();
};



#endif //__MP__RPC__APPLICATION__H__