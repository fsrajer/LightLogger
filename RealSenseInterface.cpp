#include "RealSenseInterface.h"

#ifdef WITH_REALSENSE

#include <functional>

RealSenseInterface::RealSenseInterface(int inDepthWidth,int inDepthHeight,
    int inRgbWidth,int inRgbHeight,int inFps)
  : 
  CameraInterface(inDepthWidth,inDepthHeight,inRgbWidth,inRgbHeight,inFps),
  dev(nullptr),
  initSuccessful(true)
{
  if(ctx.get_device_count() == 0)
  {
    errorText = "No device connected.";
    initSuccessful = false;
    return;
  }

  dev = ctx.get_device(0);
  dev->enable_stream(rs::stream::depth,depthWidth,depthHeight,rs::format::z16,fps);
  dev->enable_stream(rs::stream::color,rgbWidth,rgbHeight,rs::format::rgb8,fps);

  latestDepthIndex = -1;
  latestRgbIndex = -1;

  for(int i = 0; i < numBuffers; i++)
  {
    uint8_t * newImage = (uint8_t *)calloc(rgbWidth * rgbHeight* 3,sizeof(uint8_t));
    rgbBuffers[i] = std::pair<uint8_t *,int64_t>(newImage,0);
  }

  for(int i = 0; i < numBuffers; i++)
  {
    uint8_t * newDepth = (uint8_t *)calloc(depthWidth * depthHeight * 2,sizeof(uint8_t));
    uint8_t * newImage = (uint8_t *)calloc(rgbWidth * rgbHeight * 3,sizeof(uint8_t));
    frameBuffers[i] = std::pair<std::pair<uint8_t *,uint8_t *>,int64_t>(std::pair<uint8_t *,uint8_t *>(newDepth,newImage),0);
  }

  dev->set_option(rs::option::color_enable_auto_exposure,true);
  dev->set_option(rs::option::color_enable_auto_white_balance,true);

  rgbCallback = new RGBCallback(lastRgbTime,
    latestRgbIndex,
    rgbBuffers);

  depthCallback = new DepthCallback(lastDepthTime,
    latestDepthIndex,
    latestRgbIndex,
    rgbBuffers,
    frameBuffers,
    rgbWidth,
    rgbHeight);

  dev->set_frame_callback(rs::stream::depth,*depthCallback);
  dev->set_frame_callback(rs::stream::color,*rgbCallback);

  dev->start();
}

RealSenseInterface::~RealSenseInterface()
{
  if(initSuccessful)
  {
    dev->stop();

    for(int i = 0; i < numBuffers; i++)
    {
      free(rgbBuffers[i].first);
    }

    for(int i = 0; i < numBuffers; i++)
    {
      free(frameBuffers[i].first.first);
      free(frameBuffers[i].first.second);
    }

    delete rgbCallback;
    delete depthCallback;
  }
}

#endif