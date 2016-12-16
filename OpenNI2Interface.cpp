#include "OpenNI2Interface.h"

#ifdef WITH_OPENNI2

#include <functional>

using std::cout;

OpenNI2Interface::OpenNI2Interface(bool flipRows,int inDepthWidth,int inDepthHeight,
    int inRgbWidth,int inRgbHeight,int inFps)
  : 
  CameraInterface(inDepthWidth,inDepthHeight,inRgbWidth,inRgbHeight,inFps),
  flipRows(flipRows),
  initSuccessful(false)
{
  if(depthScale() != 0.001f) // sanity check
  {
    errorText = "You have changed the depth scale but you need to also change openni pixel depth format.\n";
    return;
  }

  openni::Status rc = openni::STATUS_OK;

  const char * deviceURI = openni::ANY_DEVICE;

  rc = openni::OpenNI::initialize();
  std::string errorString(openni::OpenNI::getExtendedError());
  if(errorString.length() > 0)
  {
    errorText.append(errorString);
    return;
  }

  rc = device.open(deviceURI);
  if(rc != openni::STATUS_OK)
  {
      errorText.append(openni::OpenNI::getExtendedError());
      openni::OpenNI::shutdown();
      return;
  }

  openni::VideoMode depthMode;
  depthMode.setFps(fps);
  depthMode.setPixelFormat(openni::PIXEL_FORMAT_DEPTH_1_MM);
  depthMode.setResolution(depthWidth,depthHeight);

  openni::VideoMode colorMode;
  colorMode.setFps(fps);
  colorMode.setPixelFormat(openni::PIXEL_FORMAT_RGB888);
  colorMode.setResolution(rgbWidth,rgbHeight);

  rc = depthStream.create(device,openni::SENSOR_DEPTH);
  rc = rgbStream.create(device,openni::SENSOR_COLOR);
  if(rc != openni::STATUS_OK)
  {
      errorText.append(openni::OpenNI::getExtendedError());
      openni::OpenNI::shutdown();
      return;
  }

  if(!isModeSupported(depthStream,depthMode) || !isModeSupported(rgbStream,colorMode))
  {
      errorText.append("Mode not supported.");
      cout << "Depth:\n";
      printModes(depthStream,depthMode);
      cout << "Color:\n";
      printModes(rgbStream,colorMode);
      openni::OpenNI::shutdown();
      return;
  }

  depthStream.setVideoMode(depthMode);
  rc = depthStream.start();
  if(rc != openni::STATUS_OK)
  {
      errorText.append(openni::OpenNI::getExtendedError());
      depthStream.destroy();
  }

  rgbStream.setVideoMode(colorMode);
  rc = rgbStream.start();
  if(rc != openni::STATUS_OK)
  {
      errorText.append(openni::OpenNI::getExtendedError());
      rgbStream.destroy();
  }

  if(!depthStream.isValid() || !rgbStream.isValid())
  {
      errorText.append(openni::OpenNI::getExtendedError());
      openni::OpenNI::shutdown();
      return;
  }

  latestDepthIndex = -1;
  latestRgbIndex = -1;

  for(int i = 0; i < numBuffers; i++)
  {
      uint8_t * newImage = (uint8_t *)calloc(rgbWidth * rgbHeight * 3,sizeof(uint8_t));
      rgbBuffers[i] = std::pair<uint8_t *,int64_t>(newImage,0);
  }

  for(int i = 0; i < numBuffers; i++)
  {
      uint8_t * newDepth = (uint8_t *)calloc(depthWidth * depthHeight * 2,sizeof(uint8_t));
      uint8_t * newImage = (uint8_t *)calloc(rgbWidth * rgbHeight * 3,sizeof(uint8_t));
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
      flipRows,
      rgbWidth*rgbHeight);

  depthStream.setMirroringEnabled(false);
  rgbStream.setMirroringEnabled(false);

  device.setDepthColorSyncEnabled(true);
  device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);

  rgbStream.addNewFrameListener(rgbCallback);
  depthStream.addNewFrameListener(depthCallback);  
    
  initSuccessful = true;
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

bool OpenNI2Interface::isModeSupported(const openni::VideoStream& stream,const openni::VideoMode& mode)
{
    const auto& modes = stream.getSensorInfo().getSupportedVideoModes();

    for(int i = 0; i < modes.getSize(); i++)
    {
        if(modes[i].getResolutionX() == mode.getResolutionX() &&
            modes[i].getResolutionY() == mode.getResolutionY() &&
            modes[i].getFps() == mode.getFps() &&
            modes[i].getPixelFormat() == mode.getPixelFormat())
        {
            return true;
        }
    }
    return false;
}

void OpenNI2Interface::printModes(const openni::VideoStream& stream,const openni::VideoMode& requestedMode)
{
    const auto& modes = stream.getSensorInfo().getSupportedVideoModes();

    std::cout << "Requested mode:\n";
    printMode(requestedMode);
    
    std::cout << "Supported modes:\n";
    for(int i = 0; i < modes.getSize(); i++)
    {
        printMode(modes[i]);
    }
}

void OpenNI2Interface::printMode(const openni::VideoMode& mode)
{
    std::map<int,std::string> formatNames;
    formatNames[openni::PIXEL_FORMAT_DEPTH_1_MM] = "1mm";
    formatNames[openni::PIXEL_FORMAT_DEPTH_100_UM] = "100um";
    formatNames[openni::PIXEL_FORMAT_SHIFT_9_2] = "Shift 9 2";
    formatNames[openni::PIXEL_FORMAT_SHIFT_9_3] = "Shift 9 3";

    formatNames[openni::PIXEL_FORMAT_RGB888] = "RGB888";
    formatNames[openni::PIXEL_FORMAT_YUV422] = "YUV422";
    formatNames[openni::PIXEL_FORMAT_GRAY8] = "GRAY8";
    formatNames[openni::PIXEL_FORMAT_GRAY16] = "GRAY16";
    formatNames[openni::PIXEL_FORMAT_JPEG] = "JPEG";

    cout << "(" << mode.getResolutionX() << "x" << mode.getResolutionY()
        << ", " << mode.getFps() << " fps, " << formatNames[mode.getPixelFormat()] << ")\n";
}

#endif