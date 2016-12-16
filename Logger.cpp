#include "Logger.h"

#include <iostream>

#ifdef WITH_ZLIB
#  include "zlib.h"
#endif

using std::cout;

Logger::Logger(string outDir_,std::shared_ptr<CameraInterface> cam)
  :
  doWrite(false),
  outDir(outDir_),
  fileId(0),
  writeThread(nullptr),
  cam(cam)
{
  if(!outDir.empty() && outDir[outDir.size()-1] != '/' && outDir[outDir.size()-1] != '\\')
    outDir += "/";
  depthCompressBuffer.resize(cam->depthWidth * cam->depthHeight * sizeof(uint16_t) * 4);
  rgbCompressBuffer.resize(cam->rgbWidth * cam->rgbHeight * 3 * 4);
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

  string fn = outDir + "seq" + std::to_string(fileId++) + ".klg";
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

    int32_t depthSize = cam->depthWidth * cam->depthHeight * sizeof(uint16_t);
    const void *depthData = cam->frameBuffers[bufferIdx].first.first;
    int32_t rgbSize = cam->rgbWidth * cam->rgbHeight * 3 * sizeof(uint8_t);
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

#ifdef WITH_JPEG
    std::thread rgbCompressThread(&Logger::compressJpeg,this,
      (uint8_t*)rgbData,&rgbSize);

    rgbCompressThread.join();

    rgbData = rgbCompressBuffer.data();
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

// Implemented based on example.c provided by libjpeg
void Logger::compressJpeg(const uint8_t *source,int32_t *finalSize)
{
#ifdef WITH_JPEG
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr);

  jpeg_create_compress(&cinfo);
  uint8_t *outData = &rgbCompressBuffer[0];
  unsigned long outSize = static_cast<unsigned long>(rgbCompressBuffer.size());
  jpeg_mem_dest(&cinfo,&outData,&outSize);

  cinfo.image_width = static_cast<JDIMENSION>(cam->rgbWidth);
  cinfo.image_height = static_cast<JDIMENSION>(cam->rgbHeight);
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo,90,TRUE);

  jpeg_start_compress(&cinfo,TRUE);
  JSAMPROW row_pointer[1];
  while(cinfo.next_scanline < cinfo.image_height)
  {
    row_pointer[0] = (unsigned char*)
      &source[cinfo.next_scanline * cinfo.image_width *  cinfo.input_components];
    jpeg_write_scanlines(&cinfo,row_pointer,1);
  }

  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);

  *finalSize = static_cast<int32_t>(outSize);
#endif
}