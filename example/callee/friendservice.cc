//#include <iostream>

//#include "../include/protobuf/friend.pb.h"
//#include "../../include/mprpcapplication.h"
//#include "../../include/rpcprovider.h"


//class FriendService : public fixbug::FriendServiceRpc
//{
//public:
//  std::vector<std::string> GetFriendList(uint32_t id)
//  {
//    std::cout << "do GetFriendList service\n";
//    std::vector<std::string> vec;

//    vec.push_back("liwudi");
//    vec.push_back("liwushuang");
//    vec.push_back("lishouzhong");
//    vec.push_back("lishouxin");
//    vec.push_back("lishouyi");
//    vec.push_back("lizhanshen");
//    vec.push_back("licunyi");
//    vec.push_back("licunli");

//    return vec;
//  }

//  void GetFriendList(::google::protobuf::RpcController *controller,
//                     const ::fixbug::GetFriendListRequest *req,
//                     ::fixbug::GetFriendListResponse *rps,
//                     ::google::protobuf::Closure *done)
//  {
//    uint32_t userid = req->id();
//    std::vector<std::string> friend_list = GetFriendList(userid);
//    rps->mutable_result()->set_errcode(0);
//    rps->mutable_result()->set_errmsg("");
//    // 会返回一个指针  rps->add_friends();
//    for(std::string &name : friend_list)
//    {
//      std::string *p = rps->add_friends();
//      *p = name;
//    }

//    done->Run();
//  }
//};


#include <iostream>

#include "../include/protobuf/friend.pb.h"
#include "../../include/mprpcapplication.h"
#include "../../include/rpcprovider.h"

#include "friendservice.h"

std::vector<std::string> FriendService ::GetFriendList(uint32_t id)
{
  std::cout << "do GetFriendList service\n";
  std::vector<std::string> vec;

  vec.push_back("liwudi");
  vec.push_back("liwushuang");
  vec.push_back("lishouzhong");
  vec.push_back("lishouxin");
  vec.push_back("lishouyi");
  vec.push_back("lizhanshen");
  vec.push_back("licunyi");
  vec.push_back("licunli");

  return vec;
}

void FriendService ::GetFriendList(::google::protobuf::RpcController    *controller,
                                   const ::fixbug::GetFriendListRequest *req,
                                   ::fixbug::GetFriendListResponse      *rps,
                                   ::google::protobuf::Closure          *done)
{
  uint32_t                 userid = req->id();
  std::vector<std::string> friend_list = GetFriendList(userid);
  rps->mutable_result()->set_errcode(0);
  rps->mutable_result()->set_errmsg("");
  // 会返回一个指针  rps->add_friends();
  for(std::string &name : friend_list)
  {
    std::string *p = rps->add_friends();
    *p = name;
  }

  done->Run();
}