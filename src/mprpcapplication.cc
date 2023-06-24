#include "../include/mprpcapplication.h"
#include <iostream>
#include <unistd.h>
#include <string>

#include "../include/mprpcconfig.h"

MprpcConfig MprpcApplication::config_;

static void ShowArgsHelp()
{
  std::cout << "format: command -i <config> \n";
}

void MprpcApplication::Init(int argc, char **argv)
{
  if(argc < 2)
  {
    ShowArgsHelp();
    exit(EXIT_FAILURE);
  }

  int         c = 0;
  std::string config_file;
  while((c = getopt(argc, argv, "i:")) != -1)
  {
    switch(c)
    {
      case 'i':
        config_file = optarg;
        break;
      case '?':
        ShowArgsHelp();
        exit(EXIT_FAILURE);
      case ':':
        ShowArgsHelp();
        exit(EXIT_FAILURE);
      default:
        break;
    }
  }

  // 开始加载配置文件
  // rpcserver_ip=
  // rpcserver_port=
  // zookeeper_ip=
  // zookeeper_port=
  config_.LoadConfigFile(config_file.c_str());
}


MprpcApplication &MprpcApplication::GetInstance()
{
  static MprpcApplication app;
  return app;
}

MprpcApplication::MprpcApplication() {}

MprpcApplication::~MprpcApplication() {}

MprpcConfig &MprpcApplication::GetConfig()
{
  return config_;
}