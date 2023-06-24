#include <cstdint>
#include <muduo/net/Callbacks.h>
#include <muduo/net/EventLoop.h>
#include <functional>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <google/protobuf/descriptor.h> // Member access into incomplete type 'const google::protobuf::ServiceDescriptor' (fix available)clang(incomplete_member_access) 缺少这个头文件

#include "../include/rpcprovider.h"
#include "../include/mprpcapplication.h"
#include "../include/protobuf/rpcheader.pb.h"
#include "../include/protobuf/user.pb.h"
#include "../include/zookeeperutil.h"

/*
RpcProvider    ===>


protobuf: 1. 只有数据,没有额外信息  2. 二进制存储数据  3.  不仅提供了序列化和反序列化, 还提供了rpc方法描述
*/


// 这里是框架提供给外部使用的, 可以发布 rpc 方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
  ServiceInfo service_info;
  // service 提供了哪些对象来描述服务方法
  // GetDsecript
  const google::protobuf::ServiceDescriptor *p_service_desc = service->GetDescriptor();
  // 获取服务的名字  ServiceDescriptor 类中封装了很多的方法
  std::string service_name = p_service_desc->name();

  // 获取服务对象 service 的方法的数量
  int method_cnt = p_service_desc->method_count();
  std::cout << "Service Name: " << service_name << std::endl;

  for(int i = 0; i < method_cnt; ++i)
  {
    /*
      这样存储布局
      service_name   =>   service 描述
                              ==> service *记录服务对象
                               method_name  =>  method 方法对象
    */
    // 获取了服务对象指定下标的服务方法的描述(抽象地描述)
    const google::protobuf::MethodDescriptor *p_method_desc = p_service_desc->method(i);

    std::string method_name = p_method_desc->name();
    std::cout << "Method Name: " << method_name << std::endl;

    service_info.method_map_.insert({method_name, p_method_desc});
  }

  service_info.service_ = service;
  service_map_.insert({service_name, service_info});
}

// 网络模块 绿的部分
void RpcProvider::Run()
{
  std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
  uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());

  muduo::net::InetAddress address(ip, port);
  //muduo::net::InetAddress address("127.0.0.1", 10800);
  // 组合了 TcpServer
  // 不将变量定义为成员变量, tcpserver 只在 Run() 中使用了, 不要扩展变量的范围
  muduo::net::TcpServer tcpserver(&eventloop_, address, "RpcProvider");

  // 绑定连接回调和消息读写回调方法
  tcpserver.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));

  tcpserver.setMessageCallback(
      std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

  // 设置 muduo 库的线程数量(会自动分发 IO 和 work 线程)
  tcpserver.setThreadNum(2);



  //
  // 把当前 rpc 节点上要发布的服务全部注册到 zk 上面去, 让 rpc client 可以从 zk 上发现服务
  //
  ZkClient zkCli;
  zkCli.Start();
  // service_name 为永久节点, method_name 为临时节点
  for(auto &sp : service_map_)
  {
    // service name
    std::string service_path = "/" + sp.first;
    zkCli.Create(service_path.c_str(), nullptr, 0);
    for(auto &mp : sp.second.method_map_)
    {
      // /service_name/method_name
      std::string method_path = service_path + "/" + mp.first;
      char method_path_data[128] = {0};
      sprintf(method_path_data, "%s:%d", ip.c_str(), port);
      // ZOO_EPHEMERAL 表示 znode 是一个临时节点
      zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
    }
  }

  // 启动网络服务
  tcpserver.start();

  eventloop_.loop();
}

void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
  if(!conn->connected())
  {
    // 和 rpc client 断开连接了
    conn->shutdown(); // 关闭当前连接的文件描述符
  }
}



/*
  在框架内部, RpcProvider 和 RpcConsumer 要提前协商好之间通信用的 protobuf 数据类型

  service name   method_name            args            ===> 定义 proto 的 message 类型, 进行数据的序列化和反序列化
  找对应的服务    找服务中的对应的方法     调用方法的参数       ===>  service_name, method_name, args_size
  UserServiceLoginlibingquan123465

  headr_size(4个字节) + header_str() + args_str
  16UserServiceLoginlibingquan123465

  10    "10"    在 string 中是两个字节
  10000 "10000" 在 string 中是五个字节了

  所以我们要把 int 存放 到 string 中时, 要固定为四个字节(按二进制存, 这也不会超过四个字节)
  1. 从字符串中读取一个整数
    使用 std::string 中的 insert 和 copy 方法, 当我们按内存处理数据时就非常的有用了.
*/


// 如果远端有一个 rpc 请求, 那么 onmessage 方法就会响应.
// RpcProvider  ==>  RpcServer
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buffer, muduo::Timestamp)
{
  // 从网络上接收到的远程 rpc 调用请求的字符流  Login(方法名字)  args(方法参数)
  std::string recv_buf = buffer->retrieveAllAsString();

  // 从字符流中读取前四个字节
  uint32_t header_size = 0;
  recv_buf.copy((char *)&header_size, 4, 0);

  // 根据 header_size 读取数据头的原始字符流, 然后进行反序列化, 得到 rpc 请求的详细信息
  std::string rpc_header_src = recv_buf.substr(4, header_size);

  mprpc::RpcHeader rpc_header;
  std::string service_name;
  std::string method_name;
  uint32_t args_size;

  if(rpc_header.ParseFromString(rpc_header_src))
  {
    // 数据头反序列化成功,
    service_name = rpc_header.service_name();
    method_name = rpc_header.method_name();
    args_size = rpc_header.args_size();
  }
  else
  {
    // 数据头反序列化失败, 需要记录日志
    std::cout << "rpc_header_src: " << rpc_header_src << " parse error !" << std::endl;
    return;
  }

  // 获取 rpc 方法参数的字符流数据
  std::string args_str = recv_buf.substr(4 + header_size, args_size);

  // 打印调试信息
  std::cout << "==============================================\n";
  std::cout << "header_size: " << header_size << "\n";
  std::cout << "rpc_header_src: " << rpc_header_src << "\n";
  std::cout << "service_name: " << service_name << "\n";
  std::cout << "method_name: " << method_name << "\n";
  std::cout << "args_str: " << args_str << "\n";
  std::cout << "==============================================\n";


  // 获取 service 对象和 method 对象
  auto it = service_map_.find(service_name);

  if(it == service_map_.end())
  {
    std::cout << service_name << "is not exist!\n";
    return;
  }

  auto mit = it->second.method_map_.find(method_name);

  if(mit == it->second.method_map_.end())
  {
    std::cout << service_name << ":" << method_name << "is not exist!\n";
    return;
  }

  google::protobuf::Service *service = it->second.service_;
  const google::protobuf::MethodDescriptor *method = mit->second;

  // 生成 rpc 方法调用的请求 request 和响应 response 参数
  // 使用 .New() 生成一个新的对象
  google::protobuf::Message *request = service->GetRequestPrototype(method).New();
  if(!request->ParseFromString(args_str))
  {
    std::cout << "request parse error, content: " << args_str << std::endl;
    return;
  }

  google::protobuf::Message *response = service->GetResponsePrototype(method).New();

  // 给下面的 method 方法的调用, 绑定一个 Closure 的函数回调
  // 直接这样写会报错, 不能推导出正确的类型, 在这里 再次看到了解决问题的方法, 进入到 NewCallback 方法中, 去看实际方法需要哪些参数,然后再调用
  //google::protobuf::Closure *done = google::protobuf::NewCallback<>(this, &RpcProvider::SendRpcRsponse, conn, response);
  google::protobuf::Closure *done =
      google::protobuf::NewCallback<RpcProvider, const muduo::net::TcpConnectionPtr &, google::protobuf::Message *>(
          this, &RpcProvider::SendRpcRsponse, conn, response);

  // 在框架上根据远端的 rpc 请求, 调用当前 rpc 节点上发布的方法
  // new UserService().Login(controller, request, response, done);

  // 在 callee.cc 中执行的     done->Run();  调用的就是这里绑定的回调函数
  // 这里调用的是 user.pb.h 中的 CallMethod 方法
  /*
  void UserServiceRpc::CallMethod(const ::PROTOBUF_NAMESPACE_ID::MethodDescriptor* method,
                             ::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                             const ::PROTOBUF_NAMESPACE_ID::Message* request,
                             ::PROTOBUF_NAMESPACE_ID::Message* response,
                             ::google::protobuf::Closure* done) {
  GOOGLE_DCHECK_EQ(method->service(), file_level_service_descriptors_user_2eproto[0]);
  switch(method->index()) {
    case 0:
      Login(controller,
             ::PROTOBUF_NAMESPACE_ID::internal::DownCast<const ::fixbug::LoginRequest*>(
                 request),
             ::PROTOBUF_NAMESPACE_ID::internal::DownCast<::fixbug::LoginResponse*>(
                 response),
             done);
      break;
    default:
      GOOGLE_LOG(FATAL) << "Bad method index; this should never happen.";
      break;
    }
  }
  */
  // 这个函数就是调用方法了.
  service->CallMethod(method, nullptr, request, response, done);
}

void RpcProvider::SendRpcRsponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
{
  // 黄色区域, 将响应序列化
  std::string response_str;
  if(response->SerializePartialToString(&response_str))
  {
    // 序列化成功后, 通过网络将 rpc 方法执行的结果发送给调用方
    conn->send(response_str);
    // 发送完毕后, rpc 服务主动断开连接
  }
  else
  {
    std::cout << "serialize response_str errror" << std::endl;
  }
  conn->shutdown();
}
