#ifndef LOGGER__H__
#define LOGGER__H__

#include <string>

#include "lockqueue.h"

enum class LogLevel
{
  INFO, // 普通信息
  ERROR // 错误信息
};

class Logger
{
public:
  static Logger &GetInstance();

  void Log(std::string msg);

  void SetLogLevel(LogLevel log_level);

private:
  LockQueue<std::string> lck_que_;
  std::queue<LogLevel> log_level_;

  Logger();
  ~Logger();
  Logger(const Logger &) = delete;
  Logger(Logger &&) = delete;
};


#define LOG_INFO(LogmsgFormat, ...)                                                                                    \
  do                                                                                                                   \
  {                                                                                                                    \
    Logger &logger = Logger::GetInstance();                                                                            \
    char log_msg[1024] = {0};                                                                                          \
    logger.SetLogLevel(LogLevel::INFO);                                                                                \
    snprintf(log_msg, 1016, LogmsgFormat, ##__VA_ARGS__);                                                              \
    logger.Log(log_msg);                                                                                               \
  }                                                                                                                    \
  while(0);

#define LOG_ERROR(LogmsgFormat, ...)                                                                                   \
  do                                                                                                                   \
  {                                                                                                                    \
    Logger &logger = Logger::GetInstance();                                                                            \
    char log_msg[1024] = {0};                                                                                          \
    logger.SetLogLevel(LogLevel::ERROR);                                                                               \
    snprintf(log_msg, 1015, LogmsgFormat, ##__VA_ARGS__);                                                              \
    logger.Log(log_msg);                                                                                               \
  }                                                                                                                    \
  while(0);

#endif //LOGGER__H__