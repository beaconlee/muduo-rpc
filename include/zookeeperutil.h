#ifndef ZOOKEEPER__UTIL__H__
#define ZOOKEEPER__UTIL__H__

#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>


class ZkClient
{
public:
  ZkClient();
  ~ZkClient();
  // zkclient 启动连接 zkServer
  void Start();
  // 在 zkserver 上根据指定的 path 创建 znode 节点
  void Create(const char *path, const char *data, int data_len, int status = 0);
  // 根据参数指定的 znode 节点路径, 或者 znode 节点的值
  std::string GetData(const char *path);

private:
  // zk 客户端的句柄
  zhandle_t *zhandle_t_;
};



#endif //ZOOKEEPER__UTIL__H__