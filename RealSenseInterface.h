#pragma once

#ifdef WITH_REALSENSE

#include <string>
#include <iostream>
#include <algorithm>
#include <map>
#include <atomic>

#include "librealsense/rs.hpp"

#include "CameraInterface.h"

class RealSenseInterface : public CameraInterface
{
public:
  RealSenseInterface(int width = 640,int height = 480,int fps = 30);
  virtual ~RealSenseInterface();

  virtual bool ok()
  {
    return initSuccessful;
  }

  virtual std::string error()
  {
    return errorText;
  }

  virtual float depthScale()
  {
    return dev->get_depth_scale();
  }

  struct RGBCallback
  {
  public:
    RGBCallback(int64_t & lastRgbTime,
      std::atomic<int> & latestRgbIndex,
      std::pair<uint8_t *,int64_t> * rgbBuffers)
      : lastRgbTime(lastRgbTime),
      latestRgbIndex(latestRgbIndex),
      rgbBuffers(rgbBuffers)
    {
    }

    void operator()(rs::frame frame)
    {
      lastRgbTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

      int bufferIndex = (latestRgbIndex + 1) % numBuffers;

      memcpy(rgbBuffers[bufferIndex].first,frame.get_data(),
        frame.get_width() * frame.get_height() * 3);

      rgbBuffers[bufferIndex].second = lastRgbTime;

      latestRgbIndex++;
    }

  private:
    int64_t & lastRgbTime;
    std::atomic<int> & latestRgbIndex;
    std::pair<uint8_t *,int64_t> * rgbBuffers;
  };

  struct DepthCallback
  {
  public:
    DepthCallback(int64_t & lastDepthTime,
      std::atomic<int> & latestDepthIndex,
      std::atomic<int> & latestRgbIndex,
      std::pair<uint8_t *,int64_t> * rgbBuffers,
      std::pair<std::pair<uint8_t *,uint8_t *>,int64_t> * frameBuffers)
      : lastDepthTime(lastDepthTime),
      latestDepthIndex(latestDepthIndex),
      latestRgbIndex(latestRgbIndex),
      rgbBuffers(rgbBuffers),
      frameBuffers(frameBuffers)
    {
    }

    void operator()(rs::frame frame)
    {
      lastDepthTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

      int bufferIndex = (latestDepthIndex + 1) % numBuffers;

      // The multiplication by 2 is here because the depth is actually uint16_t
      memcpy(frameBuffers[bufferIndex].first.first,frame.get_data(),
        frame.get_width() * frame.get_height() * 2);

      frameBuffers[bufferIndex].second = lastDepthTime;

      int lastImageVal = latestRgbIndex;

      if(lastImageVal == -1)
      {
        return;
      }

      lastImageVal %= numBuffers;

      memcpy(frameBuffers[bufferIndex].first.second,rgbBuffers[lastImageVal].first,
        frame.get_width() * frame.get_height() * 3);

      latestDepthIndex++;
    }

  private:
    int64_t & lastDepthTime;
    std::atomic<int> & latestDepthIndex;
    std::atomic<int> & latestRgbIndex;

    std::pair<uint8_t *,int64_t> * rgbBuffers;
    std::pair<std::pair<uint8_t *,uint8_t *>,int64_t> * frameBuffers;
  };

private:
  rs::device *dev;
  rs::context ctx;

  RGBCallback * rgbCallback;
  DepthCallback * depthCallback;

  bool initSuccessful;
  std::string errorText;
  
  std::atomic<int> latestRgbIndex;
  std::pair<uint8_t *,int64_t> rgbBuffers[numBuffers];

  int64_t lastRgbTime;
  int64_t lastDepthTime;

};
#endif