#include "Logger.h"

#include <iostream>

#ifdef WITH_ZLIB
#  include "zlib.h"
#endif

using std::cout;

Logger::Logger(string outDir,std::shared_ptr<CameraInterface> cam)
  :
  doWrite(false),
  outDir(outDir),
  fileId(0),
  writeThread(nullptr),
  cam(cam)
{
  depthCompressBuffer.resize(cam->width * cam->height * sizeof(uint16_t) * 4);
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

    int32_t depthSize = cam->width * cam->height * sizeof(uint16_t);
    const void *depthData = cam->frameBuffers[bufferIdx].first.first;
    int32_t rgbSize = cam->width * cam->height * 3 * sizeof(uint8_t);
    const void *rgbData = cam->frameBuffers[bufferIdx].first.second;

#ifdef WITH_ZLIB
    uLongf depthCompressedSize = static_cast<uLongf>(depthCompressBuffer.size());

    std::thread depthCompressThread(compress2,
      depthCompressBuffer.data(),&depthCompressedSize,
      (Bytef *)depthData,depthSize,Z_BEST_SPEED);

    depthCompressThread.join();

    depthData = depthCompressBuffer.data();
    depthSize = static_cast<int32_t>(depthCompressedSize);
#endif

    int64_t currTimestamp = cam->frameBuffers[bufferIdx].second;

    file.write((char*)&currTimestamp,sizeof(int64_t));
    file.write((char*)&depthSize,sizeof(int32_t));
    file.write((char*)&rgbSize,sizeof(int32_t));
    file.write((char*)depthData,depthSize);
    file.write((char*)rgbData,rgbSize);

    lastWrittenBufferIdx = bufferIdx;
    nFrames++;
  }
}