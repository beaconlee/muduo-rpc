//// rpc 的调用者

//#include <iostream>

//#include <muduo/net/Callbacks.h>
//#include <muduo/net/EventLoop.h>
//#include <muduo/net/InetAddress.h>
//#include <muduo/net/TcpConnection.h>
//#include <muduo/net/TcpServer.h>

//#include "../../include/mprpcapplication.h"
//#include "../../include/mprpcchannel.h"
//#include "../include/protobuf/user.pb.h"
//#include "../include/protobuf/friend.pb.h"
//#include "../include/mprpccontroller.h"


//class Server
//{
//public:
//  Server(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listen_addr, const std::string &name_arg)
//    : tcp_server_(loop, listen_addr, name_arg)
//    , loop_(loop)
//  {
//    tcp_server_.setConnectionCallback(std::bind(&Server::onConnection, this, std::placeholders::_1));

//    tcp_server_.setMessageCallback(
//        std::bind(&Server::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

//    tcp_server_.setThreadNum(2); // 1 个 IO 线程 1 个工作线程 // 输入线程数量即可，会自动分配线程
//  }

//  // 开启事件循环
//  void start() { tcp_server_.start(); }

//private:
//  muduo::net::TcpServer  tcp_server_;
//  muduo::net::EventLoop *loop_;

//  void onConnection(const muduo::net::TcpConnectionPtr &conn)
//  {
//    if(conn->connected())
//    {
//      std::cout << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort() << "state:online"
//                << std::endl;
//    }
//    else
//    {
//      std::cout << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort() << "state:offline"
//                << std::endl;
//      conn->shutdown();
//    }
//  }

//  void onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buffer, muduo::Timestamp time)
//  {
//    std::string buf = buffer->retrieveAllAsString();
//    //std::cout << "recv data: " << buf << "time: " << time.toString() << std::endl;
//    std::cout << "[time:" << time.toFormattedString() << "]"
//              << " recv data:" << buf;
//    conn->send(buf);
//  }
//};


//int main(int argc, char **argv)
//{
//  // 整个程序启动后, 想使用 mprpc 框架享受 rpc 调用, 一定要先初始化框架
//  MprpcApplication::Init(argc, argv);

//  // 定义控制对象
//  MprpcController rpc_controller;

//  // 演示调用远程发布的 rpc 方法, Login
//  /*
//  void UserServiceRpc_Stub::Login(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
//                              const ::fixbug::LoginRequest* request,
//                              ::fixbug::LoginResponse* response,
//                              ::google::protobuf::Closure* done) {
//  channel_->CallMethod(descriptor()->method(0),
//                       controller, request, response, done);
//  }
//  */

//  //
//  //  RpcController  存储一些控制信息
//  //  比如 Rpc 请求失败了, 后面的 response 就没有了, 也就不能进行访问了



//  // 这里的 RpcChannel 不是网络通道, 只是一个中介
//  fixbug::UserServiceRpc_Stub stub(new MprpcChannel());

//  // rpc 方法的请求参数  执行 Login
//  fixbug::LoginRequest request;
//  request.set_name("txwd");
//  request.set_pwd("txws");

//  // rpc 方法的响应
//  fixbug::LoginResponse response;

//  stub.Login(&rpc_controller,
//             &request,
//             &response,
//             nullptr); //RpcChannel -> RpcChannel::callMethod 集中来做所有 rpc 方法调用的参数序列化和和网络发送

//  // 这个 rpc 是一个同步的, 当 rpc 请求发送了, 程序就会阻塞住

//  // 一次 rpc 调用完成, 读 rpc 的结果
//  if(rpc_controller.Failed())
//  {
//    std::cout << rpc_controller.ErrorText() << std::endl;
//  }
//  else
//  {
//    if(0 == response.result().errcode())
//    {
//      std::cout << "rpc login reponse:" << response.sucess() << std::endl;
//    }
//    else
//    {
//      std::cout << "rpc login error:" << response.result().errmsg() << std::endl;
//    }
//  }



//  // rpc 方法的请求参数, 执行 Register
//  fixbug::RegisterRequest r_req;
//  r_req.set_id(0701);
//  r_req.set_name("tianxiawushuang");
//  r_req.set_pwd("tianxiawudi");

//  fixbug::RegisterResponse r_rps;
//  rpc_controller.Reset();
//  stub.Register(&rpc_controller,
//                &r_req,
//                &r_rps,
//                nullptr); //RpcChannel -> RpcChannel::callMethod 集中来做所有 rpc 方法调用的参数序列化和和网络发送

//  // 一次 rpc 调用完成, 读 rpc 的结果

//  if(rpc_controller.Failed())
//  {
//    std::cout << rpc_controller.ErrorText() << std::endl;
//  }
//  else
//  {
//    if(0 == r_rps.result().errcode())
//    {
//      std::cout << "rpc login reponse:" << r_rps.sucess() << std::endl;
//    }
//    else
//    {
//      std::cout << "rpc login error:" << r_rps.result().errmsg() << std::endl;
//    }
//  }


//  // rpc 方法的请求参数, 执行 Register
//  fixbug::GetFriendListRequest r_friend;
//  r_friend.set_id(0701);
//  //r_friend.set_name("tianxiawushuang");
//  //r_friend.set_pwd("tianxiawudi");

//  fixbug::FriendServiceRpc_Stub stub2(new MprpcChannel());
//  fixbug::GetFriendListResponse rs_friend;
//  rpc_controller.Reset();
//  stub2.GetFriendList(&rpc_controller,
//                      &r_friend,
//                      &rs_friend,
//                      nullptr); //RpcChannel -> RpcChannel::callMethod 集中来做所有 rpc 方法调用的参数序列化和和网络发送

//  // 一次 rpc 调用完成, 读 rpc 的结果
//  if(rpc_controller.Failed())
//  {
//    std::cout << rpc_controller.ErrorText() << std::endl;
//  }
//  else
//  {
//    if(0 == r_rps.result().errcode())
//    {
//      std::cout << "rpc GetFriend reponse:\n";
//      for(int i = 0; i < rs_friend.friends_size(); ++i)
//      {
//        std::cout << "friend name: " << rs_friend.friends(i) << std::endl;
//      }
//    }
//    else
//    {
//      std::cout << "rpc login error:" << rs_friend.result().errmsg() << std::endl;
//    }
//  }


//  return 0;
//}