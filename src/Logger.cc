#include <iostream>

#include "../include/logger.h"

#include <string>

Logger::Logger()
  : lck_que_()
  , log_level_()
{
  std::thread WriteLogTask(
      [&]()
      {
        for(;;)
        {
          // 获取当天的日期, 然后取日志信息, 写入相应的日志文件当中
          time_t now = time(nullptr);
          tm *now_tm = localtime(&now);
          char file_name[128];
          sprintf(file_name, "%d-%d-%d-log.txt", now_tm->tm_year + 1900, now_tm->tm_mon + 1, now_tm->tm_mday);
          FILE *pf = fopen(file_name, "a+");
          if(nullptr == pf)
          {
            std::cout << "logger file: " << file_name << " open error!\n";
            exit(EXIT_FAILURE);
          }

          std::string msg = lck_que_.pop();

          char time_buf[128] = {0};
          sprintf(time_buf,
                  "[%d:%d:%d] => [%s]  ",
                  now_tm->tm_hour,
                  now_tm->tm_min,
                  now_tm->tm_sec,
                  (log_level_.front() == LogLevel::INFO ? "INFO" : "ERROR"));
          msg.insert(0, time_buf);
          msg.append("\n");
          fputs(msg.c_str(), pf); // 向里面写入数据
          fclose(pf);
          log_level_.pop();
        }
      });

  //ERROR
  WriteLogTask.detach(); // 这里如果不进行线程分离的话会报错.
}

Logger::~Logger() {}

Logger &Logger::GetInstance()
{
  static Logger logger;
  return logger;
}



// 写日志, 将日志信息写入 lockqueue 缓冲队列中
void Logger::Log(std::string msg)
{
  lck_que_.push(msg);
}

void Logger::SetLogLevel(LogLevel log_level)
{
  log_level_.push(log_level);
}