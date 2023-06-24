
#include <iostream>

#include "../include/protobuf/user.pb.h"
#include "../../include/mprpcapplication.h"
#include "../../include/rpcprovider.h"
#include "friendservice.h"
#include "../../include/logger.h"


/*
UserService 原来是一个本地服务, 提供了两个进程内的本地方法, Login 和 GetFriendLists
*/

class UserService : public fixbug::UserServiceRpc // 使用在 rpc 服务的发布端
{
public:
  /*
    virtual void Login(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::fixbug::LoginRequest* request,
                       ::fixbug::LoginResponse* response,
                       ::google::protobuf::Closure* done);
  */
  bool Login(std::string name, std::string pwd)
  {
    std::cout << "doing local service: Login\n";
    std::cout << "name: " << name << "  pwd:" << pwd << "\n";
    return true;
  }

  // 重写基类 UserService 的虚函数, 下面的这些方法都是框架直接调用的
  // 我们现在是服务的发布者
  // 1. caller  ==>  发起一个 rpc 请求 Login(LoginRequest) => muduo => callee
  // 2. callee  ==>  远端想调用 Login 方法   ==>  Login(LoginRequest)  ==>  交到下面重写的这个 Login 方法上面
  void Login(::google::protobuf::RpcController *controller,
             const ::fixbug::LoginRequest *request,
             ::fixbug::LoginResponse *response,
             ::google::protobuf::Closure *done)
  {
    // 框架给业务上报了请求参数  LoginRequest, 应用程序获取相应数据做本地业务
    std::string name = request->name();
    std::string pwd = request->pwd();

    // 然后调用本地业务方法
    bool login_result = Login(name, pwd);

    // 本地业务做完后, 把响应写入
    response->set_sucess(login_result);

    fixbug::ResultCode *code = response->mutable_result();
    code->set_errcode(0);
    code->set_errmsg("");

    // 执行回调操作, 执行响应对象数据的序列化和网络发送(都是由框架完成的)
    // Closure 是一个抽象类
    done->Run();
  }


  // 当我们在 proto 中增加了 Register 方法之后, fixbug::UserServiceRpc 不仅有 Login 方法, 还有 Register 方法
  /*
    virtual void Register(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::fixbug::RegisterRequest* request,
                       ::fixbug::RegisterResponse* response,
                       ::google::protobuf::Closure* done);
  */
  // 本地注册方法
  bool Register(std::string name, std::string pwd)
  {
    std::cout << " 执行本地Register方法: "
              << "[name=" << name << "] [pws=" << pwd << "]\n";
    return true;
  }

  void Register(::google::protobuf::RpcController *controller,
                const ::fixbug::RegisterRequest *request,
                ::fixbug::RegisterResponse *response,
                ::google::protobuf::Closure *done)
  {
    uint32_t id = request->id();
    std::string user_name = request->name();
    std::string user_pwd = request->pwd();

    bool ret = Register(user_name, user_pwd);

    response->mutable_result()->set_errcode(0);
    response->mutable_result()->set_errmsg("");
    response->set_sucess(ret);

    done->Run();
  }
};

class FriendService;


int main(int argc, char **argv)
{
  LOG_INFO("First log message%s", "wudi");
  LOG_ERROR("%s : %s : %d", __FILE__, __FUNCTION__, __LINE__);

  // 调用框架的初始化操作
  MprpcApplication::Init(argc, argv);
  // provider 是一个 rpc 网络服务对象, 把 UserService 对象发布到 rpc 节点
  RpcProvider provider; // 发布服务

  // 将 UserService 对象发布到 rpc 节点上
  provider.NotifyService(new UserService());
  provider.NotifyService(new FriendService());
  //provider.NotifyService(XXXXXX);

  // 启动一个 rpc 服务发布节点
  provider.Run();

  return 0;
}



/*
       发布 RPC 方法步骤:

       1. 定义 proto 文件描述
       2. 在 callee 中实现本地方法 XXX,
       3. 重写 fixbug::XXXRpc 方法
*/


