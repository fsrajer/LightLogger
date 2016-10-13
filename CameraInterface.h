#pragma once

#include <string>
#include <iostream>
#include <algorithm>
#include <map>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

class CameraInterface
{
    public:
      CameraInterface(int width,int height,int fps)
        : width(width),height(height),fps(fps)
      {
      }

      virtual ~CameraInterface() {}

      virtual bool ok() = 0;
      virtual std::string error() = 0;
      virtual float depthScale() = 0;

      const int width,height,fps;
      static const int numBuffers = 10;
      std::atomic<int> latestDepthIndex;
      std::pair<std::pair<uint8_t *,uint8_t *>,int64_t> frameBuffers[numBuffers];
};
