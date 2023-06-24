#ifndef __RPC__PROVIDER__H__
#define __RPC__PROVIDER__H__



#include <google/protobuf/service.h>
#include <muduo/base/Timestamp.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <string>

class RpcProvider
{
public:
  // 这里是框架提供给外部使用的, 可以发布 rpc 方法的函数接口
  // 这里不能接收具体的类指针
  void NotifyService(google::protobuf::Service *service);

  // 启动 rpc 服务节点, 开始提供 rpc 远程网络调用服务
  void Run();


private:
  // 组合了 EventLoop
  muduo::net::EventLoop eventloop_;

  // 服务类型信息
  struct ServiceInfo
  {
    google::protobuf::Service *service_;  // 保存服务对象
    std::unordered_map<std::string, const google::protobuf::MethodDescriptor *> method_map_; // 保存服务方法
  };

  // 存储注册成功的服务对象, 和其服务方法的所有信息
  std::unordered_map<std::string, ServiceInfo> service_map_;

  // 新的 socket 连接的回调
  void OnConnection(const muduo::net::TcpConnectionPtr &);
  // 已建立连接用户的读写事件回调
  void OnMessage(const muduo::net::TcpConnectionPtr &, muduo::net::Buffer *, muduo::Timestamp);

  // Closer 的回调操作, 用于序列化 rpc 的响应 和 网络发送
  void SendRpcRsponse(const muduo::net::TcpConnectionPtr &, google::protobuf::Message *);
};


#endif //__RPC__PROVIDER__H__