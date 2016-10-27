#include "OpenNI2Interface.h"

#ifdef WITH_OPENNI2

#include <functional>

OpenNI2Interface::OpenNI2Interface(bool flipRows,int inWidth,int inHeight,int inFps)
  : 
  CameraInterface(inWidth,inHeight,inFps),
  flipRows(flipRows),
  initSuccessful(true)
{
  if(depthScale() != 0.001f) // sanity check
  {
    errorText = "You have changed the depth scale but you need to also change openni pixel depth format.\n";
    initSuccessful = false;
    return;
  }

  openni::Status rc = openni::STATUS_OK;

  const char * deviceURI = openni::ANY_DEVICE;

  rc = openni::OpenNI::initialize();

  std::string errorString(openni::OpenNI::getExtendedError());

  if(errorString.length() > 0)
  {
    errorText.append(errorString);
    initSuccessful = false;
  } else
  {
    rc = device.open(deviceURI);
    if(rc != openni::STATUS_OK)
    {
      errorText.append(openni::OpenNI::getExtendedError());
      openni::OpenNI::shutdown();
      initSuccessful = false;
    } else
    {
      openni::VideoMode depthMode;
      depthMode.setFps(fps);
      depthMode.setPixelFormat(openni::PIXEL_FORMAT_DEPTH_1_MM);
      depthMode.setResolution(width,height);

      openni::VideoMode colorMode;
      colorMode.setFps(fps);
      colorMode.setPixelFormat(openni::PIXEL_FORMAT_RGB888);
      colorMode.setResolution(width,height);

      rc = depthStream.create(device,openni::SENSOR_DEPTH);
      if(rc == openni::STATUS_OK)
      {
        depthStream.setVideoMode(depthMode);
        rc = depthStream.start();
        if(rc != openni::STATUS_OK)
        {
          errorText.append(openni::OpenNI::getExtendedError());
          depthStream.destroy();
          initSuccessful = false;
        }
      } else
      {
        errorText.append(openni::OpenNI::getExtendedError());
        initSuccessful = false;
      }

      rc = rgbStream.create(device,openni::SENSOR_COLOR);
      if(rc == openni::STATUS_OK)
      {
        rgbStream.setVideoMode(colorMode);
        rc = rgbStream.start();
        if(rc != openni::STATUS_OK)
        {
          errorText.append(openni::OpenNI::getExtendedError());
          rgbStream.destroy();
          initSuccessful = false;
        }
      } else
      {
        errorText.append(openni::OpenNI::getExtendedError());
        initSuccessful = false;
      }

      if(!depthStream.isValid() || !rgbStream.isValid())
      {
        errorText.append(openni::OpenNI::getExtendedError());
        openni::OpenNI::shutdown();
        initSuccessful = false;
      }

      if(initSuccessful)
      {
        latestDepthIndex = -1;
        latestRgbIndex = -1;

        for(int i = 0; i < numBuffers; i++)
        {
          uint8_t * newImage = (uint8_t *)calloc(width * height * 3,sizeof(uint8_t));
          rgbBuffers[i] = std::pair<uint8_t *,int64_t>(newImage,0);
        }

        for(int i = 0; i < numBuffers; i++)
        {
          uint8_t * newDepth = (uint8_t *)calloc(width * height * 2,sizeof(uint8_t));
          uint8_t * newImage = (uint8_t *)calloc(width * height * 3,sizeof(uint8_t));
          frameBuffers[i] = std::pair<std::pair<uint8_t *,uint8_t *>,int64_t>(std::pair<uint8_t *,uint8_t *>(newDepth,newImage),0);
        }

        rgbCallback = new RGBCallback(lastRgbTime,
          latestRgbIndex,
          rgbBuffers,
          flipRows);

        depthCallback = new DepthCallback(lastDepthTime,
          latestDepthIndex,
          latestRgbIndex,
          rgbBuffers,
          frameBuffers,
          flipRows);

        depthStream.setMirroringEnabled(false);
        rgbStream.setMirroringEnabled(false);

        device.setDepthColorSyncEnabled(true);
        device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);

        rgbStream.addNewFrameListener(rgbCallback);
        depthStream.addNewFrameListener(depthCallback);
      }
    }
  }
}

OpenNI2Interface::~OpenNI2Interface()
{
  if(initSuccessful)
  {
    rgbStream.removeNewFrameListener(rgbCallback);
    depthStream.removeNewFrameListener(depthCallback);

    depthStream.stop();
    rgbStream.stop();
    depthStream.destroy();
    rgbStream.destroy();
    device.close();
    openni::OpenNI::shutdown();

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