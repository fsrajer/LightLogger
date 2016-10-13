#pragma once

#include <string>
#include <atomic>
#include <cstdint>
#include <fstream>
#include <thread>

class Logger
{
public:
  Logger(string outDir);
  ~Logger();

  bool isWriting() const
  {
    return doWrite;
  }

  void startWriting();
  void stopWriting();

private:
  void write();

  std::atomic<bool> doWrite;
  string outDir;
  int64_t lastTimestamp;
  int32_t nFrames;
  int fileId;
  std::ofstream file;
  std::thread *writeThread;
};
