#include "../include/mprpcconfig.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>

void MprpcConfig::LoadConfigFile(const char *config_file)
{
  FILE *pf = fopen(config_file, "r");
  if(nullptr == pf)
  {
    std::cout << config_file << "is not exist\n";
    exit(EXIT_FAILURE);
  }

  // 1. 注释  2. 正确的配置项  3. 去掉开头多余的空格
  while(!feof(pf))
  {
    char buf[512] = {0};
    fgets(buf, 512, pf);

    std::string read_buf(buf);

    Trim(read_buf);

    // 判断 # 的注释
    if(read_buf[0] == '#' || read_buf.empty())
    {
      continue;
    }

    // 解析配置项
    int idx = read_buf.find('=');
    if(-1 == idx)
    {
      continue;
    }

    std::string key;
    std::string value;
    key = read_buf.substr(0, idx);
    Trim(key);
    int endidx = read_buf.find('\n', idx);
    // 这里之所以是endidx - idx - 1, 其实是 endidx - (idx + 1)
    // 我们是从 idx + 1 开始读取的
    value = read_buf.substr(idx + 1, endidx - idx - 1);
    Trim(value);
    config_map_[key] = value;
  }
}

std::string MprpcConfig::Load(const std::string &key)
{
  auto it = config_map_.find(key);
  if(it == config_map_.end())
  {
    return "";
  }

  return it->second;
}

void MprpcConfig::Trim(std::string &src_buf)
{
  // 去掉字符串前面多余的空格

  int idx = src_buf.find_first_not_of(' ');
  if(-1 != idx)
  {
    // 说明前面有空格
    src_buf = src_buf.substr(idx, src_buf.size() - idx);
  }

  // 去掉字符串后面多余的空格
  idx = src_buf.find_last_not_of(' ');
  if(-1 != idx)
  {
    // idx 表示的是长度, 下面的是下标, 所以要加以, 在 C++ 中这些括号都是左闭右开的
    src_buf = src_buf.substr(0, idx + 1);
  }
}