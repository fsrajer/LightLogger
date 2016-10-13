#include "Logger.h"

#include <iostream>

using std::cout;

Logger::Logger()
  : doWrite(false)
{
}

void Logger::startWriting()
{
  cout << "Starting writing.\n";
  doWrite = true;
}

void Logger::stopWriting()
{
  cout << "Stopping writing.\n";
  doWrite = false;
}