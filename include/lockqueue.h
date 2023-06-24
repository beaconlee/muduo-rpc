#ifndef LOCK__QUEUE__H__
#define LOCK__QUEUE__H__


#include <queue>
#include <thread>
#include <mutex>              // 线程互斥
#include <condition_variable> // 线程间通信

template <typename T> class LockQueue
{
public:
  void push(const T &data)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(data);
    condvariable_.notify_one(); // 只有一个线程进行写
  }

  T pop()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    while(queue_.empty())
    {
      // 日志队列为空,线程进入 wait 状态
      condvariable_.wait(lock);
    }
    T data = queue_.front();
    queue_.pop();
    return data;
  }

private:
  std::queue<T> queue_;
  std::mutex mutex_;

  std::condition_variable condvariable_;
};


#endif //LOCK__QUEUE__H__