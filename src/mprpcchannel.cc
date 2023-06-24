#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <unistd.h>

#include "protobuf/rpcheader.pb.h"
#include "../include/mprpcchannel.h"
#include "../include/zookeeperutil.h"

void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                              google::protobuf::RpcController          *controller,
                              const google::protobuf::Message          *request,
                              google::protobuf::Message                *response,
                              google::protobuf::Closure                *done)
{
  /* header_size + service_name method_name args_size + args */
  const google::protobuf::ServiceDescriptor *sd = method->service();

  std::string service_name = sd->name();
  std::string method_name = method->name();

  std::string args_str;
  int         args_size = 0;
  if(request->SerializeToString(&args_str))
  {
    args_size = args_str.size();
  }
  else
  {
    controller->SetFailed("serialize request error!");
    return;
  }

  // 定义 rpc 的header
  mprpc::RpcHeader rpc_header;
  rpc_header.set_service_name(service_name);
  rpc_header.set_method_name(method_name);
  rpc_header.set_args_size(args_size);

  uint32_t    header_size = 0;
  std::string rpc_header_str;
  if(rpc_header.SerializeToString(&rpc_header_str))
  {
    header_size = rpc_header_str.size();
  }
  else
  {
    controller->SetFailed("serialize rpc header error");
    return;
  }

  // 组织待发送的 rpc 请求的字符串
  std::string send_rpc_str;
  //send_rpc_str.insert(0, std::string(static_cast<char *>(&header_size), 4));
  send_rpc_str.insert(0, std::string{(char *)&header_size, 4});
  send_rpc_str += rpc_header_str;
  send_rpc_str += args_str;

  // 打印调试信息
  std::cout << "==============================================\n";
  std::cout << "header_size: " << header_size << "\n";
  std::cout << "rpc_header_str: " << rpc_header_str << "\n";
  std::cout << "service_name: " << service_name << "\n";
  std::cout << "method_name: " << method_name << "\n";
  std::cout << "args_str: " << args_str << "\n";
  std::cout << "==============================================\n";

  // 使用tcp编程, 完成rpc方法的远程调用
  int clientfd = socket(AF_INET, SOCK_STREAM, 0);

  if(-1 == clientfd)
  {
    char errtxt[512] = {0};
    sprintf(errtxt, "create socket error! errno: %d", errno);
    controller->SetFailed(errtxt);
    return;
  }

  // 读取配置文件 rpcservice 的信息

  // std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
  // uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());


  // 现在不用静态的查询 服务的 ip 和 port 了, 可以从 zk 上动态的获取 ip 和 port
  // rpc 调用方想调用 service_name 的 method_name 服务, 需要查询 zk 上该服务的 host 信息

  ZkClient zkCli;
  zkCli.Start();
  std::string method_path = "/" + service_name + "/" + method_name;
  std::string host_data = zkCli.GetData(method_path.c_str());

  if (host_data == "")
  {
  	controller->SetFailed(method_path + "is not exist!");
  	return;
  }
  int idx = host_data.find(":");
  if (idx == -1)
  {
  	controller->SetFailed(method_path + "address is invalid!");
  	return;
  }

  // ip 和 端口号现在 不是读取的配置文件中的了, 而是从 zookeeper 中找到 znode, 然后从 znode 中获取到的 IP 和 port
  std::string ip = host_data.substr(0, idx);
  uint16_t port = atoi(host_data.substr(idx + 1, host_data.size() - idx).c_str());

  sockaddr_in addr;

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ip.c_str()); //  INADDR_ANY;

  socklen_t len = sizeof(sockaddr_in);

  // 连接rpc服务节点
  if(-1 == connect(clientfd, (sockaddr *)&addr, len))
  {
    close(clientfd);
    char errtxt[512] = {0};
    sprintf(errtxt, "connect socket error! errno: %d", errno);
    controller->SetFailed(errtxt);
    return;
  }

  // 发送rpc请求
  if(-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
  {
    close(clientfd);
    char errtxt[512] = {0};
    sprintf(errtxt, "send socket error! errno: %d", errno);
    controller->SetFailed(errtxt);
    return;
  }

  // 接收rpc请求的响应值
  char recv_buf[1024] = {0};
  int  recv_size = 0;
  if(-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0)))
  {
    close(clientfd);
    char errtxt[512] = {0};
    sprintf(errtxt, "recv socket error! errno: %d", errno);
    controller->SetFailed(errtxt);
    return;
  }

  // 反序列化rpc调用的响应数据
  // std::string response_str(recv_buf, 0, recv_size);
  // bug出现问题了, recv_buf遇到\0后面的数据就存不下来了
  // if (response->ParseFromString(response_str))
  // 不要使用 ParseFromString  使用 ParseFromArray

  if(!response->ParseFromArray(recv_buf, recv_size))
  {
    close(clientfd);
    char errtxt[2046] = {0};
    sprintf(errtxt, "parse error! response_str: %s", recv_buf);
    controller->SetFailed(errtxt);
    return;
  }
}