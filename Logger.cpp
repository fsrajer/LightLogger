#include "Logger.h"

#include <iostream>

using std::cout;

Logger::Logger(string outDir)
  :
  doWrite(false),
  outDir(outDir),
  fileId(0),
  writeThread(nullptr)
{
}

Logger::~Logger()
{
  if(doWrite)
  {
    stopWriting();
  }
}

void Logger::startWriting()
{
  cout << "Starting writing.\n";
  doWrite = true;
  lastTimestamp = -1;
  nFrames = 0;

  string fn = outDir + "/seq" + std::to_string(fileId++) + ".klg";
  file.open(fn,std::ios::binary);
  if(!file.is_open())
  {
    cout << "Could not open file: " << fn << "\n";
    doWrite = false;
    return;
  }

  file.write((char*)&nFrames,sizeof(int32_t));
  writeThread = new std::thread(&Logger::write,this);
}

void Logger::stopWriting()
{
  cout << "Stopping writing.\n";
  doWrite = false;
  writeThread->join();
  delete writeThread;
  writeThread = nullptr;

  file.seekp(0);
  file.write((char*)&nFrames,sizeof(int32_t));

  file.close();
  file.clear();
}

void Logger::write()
{
  cout << "Writing.\n";
}