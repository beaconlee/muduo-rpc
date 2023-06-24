// rpc 的调用者

#include <iostream>

#include <muduo/net/Callbacks.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/TcpServer.h>

#include "../../include/mprpcapplication.h"
#include "../../include/mprpcchannel.h"
#include "../../include/protobuf/user.pb.h"
#include "../../include/protobuf/friend.pb.h"
#include "../../include/mprpccontroller.h"


MprpcController rpc_controller;

class Server
{
public:
  Server(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listen_addr, const std::string &name_arg)
    : tcp_server_(loop, listen_addr, name_arg)
    , loop_(loop)
  {
    tcp_server_.setConnectionCallback(std::bind(&Server::onConnection, this, std::placeholders::_1));

    tcp_server_.setMessageCallback(
        std::bind(&Server::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    tcp_server_.setThreadNum(2); // 1 个 IO 线程 1 个工作线程 // 输入线程数量即可，会自动分配线程
  }

  // 开启事件循环
  void start() { tcp_server_.start(); }

private:
  muduo::net::TcpServer  tcp_server_;
  muduo::net::EventLoop *loop_;

  void onConnection(const muduo::net::TcpConnectionPtr &conn)
  {
    if(conn->connected())
    {
      std::cout << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort() << "state:online"
                << std::endl;
    }
    else
    {
      std::cout << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort() << "state:offline"
                << std::endl;
      conn->shutdown();
    }
  }

  void onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buffer, muduo::Timestamp time)
  {
    std::string buf = buffer->retrieveAllAsString();
    int         index = buf.find_first_of('\n');
    buf = buf.substr(0, index);
    index = buf.find_first_of('\r');
    buf = buf.substr(0, index);
    std::cout << "buf:" << buf << std::endl;
    std::cout << "buf.size()" << buf.size() << std::endl;
    if(buf == "login")
    {
      fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
      // rpc 方法的请求参数  执行 Login
      fixbug::LoginRequest        request;
      request.set_name("txwd");
      request.set_pwd("txws");

      // rpc 方法的响应
      fixbug::LoginResponse response;

      stub.Login(&rpc_controller,
                 &request,
                 &response,
                 nullptr); //RpcChannel -> RpcChannel::callMethod 集中来做所有 rpc 方法调用的参数序列化和和网络发送

      // 这个 rpc 是一个同步的, 当 rpc 请求发送了, 程序就会阻塞住

      // 一次 rpc 调用完成, 读 rpc 的结果
      if(rpc_controller.Failed())
      {
        std::cout << rpc_controller.ErrorText() << std::endl;
      }
      else
      {
        if(0 == response.result().errcode())
        {
          std::cout << "rpc login reponse:" << response.sucess() << std::endl;
          conn->send("rpc login reponse: %s", response.sucess());
        }
        else
        {
          std::cout << "rpc login error:" << response.result().errmsg() << std::endl;
        }
      }
    }
    else if(buf == "register")
    {
      fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
      // rpc 方法的请求参数, 执行 Register
      fixbug::RegisterRequest     r_req;
      r_req.set_id(0701);
      r_req.set_name("tianxiawushuang");
      r_req.set_pwd("tianxiawudi");

      fixbug::RegisterResponse r_rps;
      rpc_controller.Reset();
      stub.Register(&rpc_controller,
                    &r_req,
                    &r_rps,
                    nullptr); //RpcChannel -> RpcChannel::callMethod 集中来做所有 rpc 方法调用的参数序列化和和网络发送

      // 一次 rpc 调用完成, 读 rpc 的结果

      if(rpc_controller.Failed())
      {
        std::cout << rpc_controller.ErrorText() << std::endl;
      }
      else
      {
        if(0 == r_rps.result().errcode())
        {
          std::cout << "rpc register reponse:" << r_rps.sucess() << std::endl;
          conn->send("rpc register reponse: %s", r_rps.sucess());
        }
        else
        {
          std::cout << "rpc register error:" << r_rps.result().errmsg() << std::endl;
        }
      }
    }
    else if(buf == "getfriends")
    {
      fixbug::GetFriendListRequest r_friend;
      r_friend.set_id(0701);
      //r_friend.set_name("tianxiawushuang");
      //r_friend.set_pwd("tianxiawudi");

      fixbug::FriendServiceRpc_Stub stub2(new MprpcChannel());
      fixbug::GetFriendListResponse rs_friend;
      rpc_controller.Reset();
      stub2.GetFriendList(
          &rpc_controller,
          &r_friend,
          &rs_friend,
          nullptr); //RpcChannel -> RpcChannel::callMethod 集中来做所有 rpc 方法调用的参数序列化和和网络发送

      // 一次 rpc 调用完成, 读 rpc 的结果
      if(rpc_controller.Failed())
      {
        std::cout << rpc_controller.ErrorText() << std::endl;
      }
      else
      {
        if(0 == rs_friend.result().errcode())
        {
          std::cout << "rpc GetFriend reponse:\n";
          for(int i = 0; i < rs_friend.friends_size(); ++i)
          {
            std::cout << "friend name: " << rs_friend.friends(i) << std::endl;
            conn->send(rs_friend.friends(i));
          }
        }
        else
        {
          std::cout << "rpc login error:" << rs_friend.result().errmsg() << std::endl;
        }
      }
    }
    else
    {
      std::cout << " request error, please again.\n";
    }
  }
};


int main(int argc, char **argv)
{
  // 整个程序启动后, 想使用 mprpc 框架享受 rpc 调用, 一定要先初始化框架
  MprpcApplication::Init(argc, argv);

  muduo::net::EventLoop   loop;
  muduo::net::InetAddress addr("127.0.0.1", 10700);
  Server                  ser(&loop, addr, "WUDI");

  ser.start();
  loop.loop(); // epoll_wait
  return 0;

  return 0;
}