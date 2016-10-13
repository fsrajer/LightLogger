#pragma once

#include <atomic>

class Logger
{
public:
  Logger();

  bool isWriting() const
  {
    return doWrite;
  }

  void startWriting();
  void stopWriting();

private:
  std::atomic<bool> doWrite;
};
