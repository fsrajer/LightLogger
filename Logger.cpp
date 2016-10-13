#include "Logger.h"

#include <iostream>

using std::cout;

Logger::Logger(string outDir,std::shared_ptr<CameraInterface> cam)
  :
  doWrite(false),
  outDir(outDir),
  fileId(0),
  writeThread(nullptr),
  cam(cam)
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
  static int lastWrittenBufferIdx = -1;
  while(doWrite)
  {
    std::this_thread::sleep_for(std::chrono::microseconds(1000));

    int lastDepth = cam->latestDepthIndex;
    if(lastDepth == -1)
      continue;

    int bufferIdx = lastDepth % CameraInterface::numBuffers;
    if(bufferIdx == lastWrittenBufferIdx)
      continue;
    
    const void *depthData = cam->frameBuffers[bufferIdx].first.first;
    const void *rgbData = cam->frameBuffers[bufferIdx].first.second;

    int64_t currTimestamp = cam->frameBuffers[bufferIdx].second;
    int32_t depthSize = cam->width * cam->height * sizeof(uint16_t);
    int32_t rgbSize = cam->width * cam->height * 3 * sizeof(uint8_t);

    file.write((char*)&currTimestamp,sizeof(int64_t));
    file.write((char*)&depthSize,sizeof(int32_t));
    file.write((char*)&rgbSize,sizeof(int32_t));
    file.write((char*)depthData,depthSize);
    file.write((char*)rgbData,rgbSize);

    lastWrittenBufferIdx = bufferIdx;
    nFrames++;
  }
}