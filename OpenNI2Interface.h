#pragma once

#ifdef WITH_OPENNI2

#include <string>
#include <iostream>
#include <algorithm>
#include <map>
#include <atomic>

#include <OpenNI.h>

#include "CameraInterface.h"

typedef struct { uint8_t r,g,b; } ColorType;

template<typename T>
void copyWithReversedRows(void *destination,const void* source,int height,int width)
{
  // rowwise
  T *dst = (T*)destination;
  const T* src = (const T*)source;
  
  for(int i = 0; i < height; i++)
  {
    for(int j = 0; j < width; j++)
    {
      dst[width*i+(width-j-1)] = src[width*i+j];
    }
  }
}

class OpenNI2Interface : public CameraInterface
{
public:
  OpenNI2Interface(bool flipRows = false,int width = 640,int height = 480,int fps = 30);
  virtual ~OpenNI2Interface();

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
    return 0.001f;
  }
  
  class RGBCallback : public openni::VideoStream::NewFrameListener
  {
  public:
    RGBCallback(int64_t & lastRgbTime,
      std::atomic<int> & latestRgbIndex,
      std::pair<uint8_t *,int64_t> * rgbBuffers,
      bool flipRows)
      : lastRgbTime(lastRgbTime),
      latestRgbIndex(latestRgbIndex),
      rgbBuffers(rgbBuffers),
      flipRows(flipRows)
    {
    }

    virtual ~RGBCallback() {}

    void onNewFrame(openni::VideoStream& stream)
    {
      stream.readFrame(&frame);

      lastRgbTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

      int bufferIndex = (latestRgbIndex + 1) % numBuffers;

      if(flipRows)
        copyWithReversedRows<ColorType>(rgbBuffers[bufferIndex].first,frame.getData(),
        frame.getHeight(),frame.getWidth());
      else
        memcpy(rgbBuffers[bufferIndex].first,frame.getData(),frame.getHeight() * frame.getWidth() * 3);

      rgbBuffers[bufferIndex].second = lastRgbTime;

      latestRgbIndex++;
    }

  private:
    openni::VideoFrameRef frame;
    int64_t & lastRgbTime;
    std::atomic<int> & latestRgbIndex;
    std::pair<uint8_t *,int64_t> * rgbBuffers;
    bool flipRows;
  };

  class DepthCallback : public openni::VideoStream::NewFrameListener
  {
  public:
    DepthCallback(int64_t & lastDepthTime,
      std::atomic<int> & latestDepthIndex,
      std::atomic<int> & latestRgbIndex,
      std::pair<uint8_t *,int64_t> * rgbBuffers,
      std::pair<std::pair<uint8_t *,uint8_t *>,int64_t> * frameBuffers,
      bool flipRows)
      : lastDepthTime(lastDepthTime),
      latestDepthIndex(latestDepthIndex),
      latestRgbIndex(latestRgbIndex),
      rgbBuffers(rgbBuffers),
      frameBuffers(frameBuffers),
      flipRows(flipRows)
    {
    }

    virtual ~DepthCallback() {}

    void onNewFrame(openni::VideoStream& stream)
    {
      stream.readFrame(&frame);

      lastDepthTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

      int bufferIndex = (latestDepthIndex + 1) % numBuffers;

      if(flipRows)
        copyWithReversedRows<uint16_t>(frameBuffers[bufferIndex].first.first,frame.getData(),
        frame.getHeight(),frame.getWidth());
      else
        memcpy(frameBuffers[bufferIndex].first.first,frame.getData(),frame.getHeight()*frame.getWidth()*2);

      frameBuffers[bufferIndex].second = lastDepthTime;

      int lastImageVal = latestRgbIndex;

      if(lastImageVal == -1)
      {
        return;
      }

      lastImageVal %= numBuffers;

      memcpy(frameBuffers[bufferIndex].first.second,rgbBuffers[lastImageVal].first,frame.getWidth() * frame.getHeight() * 3);

      latestDepthIndex++;
    }

  private:
    openni::VideoFrameRef frame;
    int64_t & lastDepthTime;
    std::atomic<int> & latestDepthIndex;
    std::atomic<int> & latestRgbIndex;

    std::pair<uint8_t *,int64_t> * rgbBuffers;
    std::pair<std::pair<uint8_t *,uint8_t *>,int64_t> * frameBuffers;
    bool flipRows;
  };

private:
  const bool flipRows;

  openni::Device device;

  openni::VideoStream depthStream;
  openni::VideoStream rgbStream;

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