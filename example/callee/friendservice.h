
#include "../include/protobuf/friend.pb.h"


class FriendService : public fixbug::FriendServiceRpc
{
public:
  std::vector<std::string> GetFriendList(uint32_t id);

  void GetFriendList(::google::protobuf::RpcController *controller,
                     const ::fixbug::GetFriendListRequest *req,
                     ::fixbug::GetFriendListResponse *rps,
                     ::google::protobuf::Closure *done);
};