#ifndef MP__RPC__CONFIG__H__
#define MP__RPC__CONFIG__H__


#include <unordered_map>
#include <string>


//
// 框架读取配置文件类
// rpcserverip  rpcserverport  zookeeperip  zookeeperport
//
class MprpcConfig
{
public:
  void        LoadConfigFile(const char *config_file);
  std::string Load(const std::string &key);

private:
  std::unordered_map<std::string, std::string> config_map_;

  // 去掉字符串前后的空格
  void Trim(std::string &src_buf);
};

#endif //MP__RPC__CONFIG__H__