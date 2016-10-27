#include <iostream>
#include <memory>
#include <string>
#include <cstdint>

#include "GL/glut.h"
#include "GL/freeglut.h"

#include "CameraInterface.h"
#include "RealSenseInterface.h"
#include "OpenNI2Interface.h"
#include "Logger.h"

using std::cout;
using std::string;

const static int width = 640;
const static int height = 480;
const static int fps = 30;
const static bool flipRows = true; // mirroring every row - for openni only
std::shared_ptr<CameraInterface> cam;
std::unique_ptr<Logger> logger;

void displayCallback()
{
  int lastDepth = cam->latestDepthIndex;
  if(lastDepth == -1)
  {
    return;
  }
  int bufferIdx = lastDepth % CameraInterface::numBuffers;
  const void *depthData = cam->frameBuffers[bufferIdx].first.first;
  const void *rgbData = cam->frameBuffers[bufferIdx].first.second;

  glClear(GL_COLOR_BUFFER_BIT);
  glPixelZoom(1,-1);

  // Display depth data by linearly mapping depth between 0 and 2 meters to the red channel
  glRasterPos2f(-1,1);
  glPixelTransferf(GL_RED_SCALE,0xFFFF * cam->depthScale() / 2.0f);
  glDrawPixels(width,height,GL_RED,GL_UNSIGNED_SHORT,depthData);
  glPixelTransferf(GL_RED_SCALE,1.0f);

  // Display color image as RGB triples
  glRasterPos2f(0,1);
  glDrawPixels(width,height,GL_RGB,GL_UNSIGNED_BYTE,rgbData);

  glutSwapBuffers();
}
void idleCallback()
{
  glutPostRedisplay();
}

void endApp()
{
  glutLeaveMainLoop();
  if(logger->isWriting())
  {
    logger->stopWriting();
  }
}

void keyboardCallback(unsigned char keyPressed,int mouseX,int mouseY)
{
  switch(keyPressed)
  {
  case 27:
    endApp();
    break;
  case 'r':
    if(logger->isWriting())
      logger->stopWriting();
    else
      logger->startWriting();
    break;
  default:
    break;
  }
}

int main(int argc,char *argv[])
{
  string outDir = "";
  if(argc > 1)
    outDir = argv[1];

#ifdef WITH_REALSENSE
  cam = std::make_shared<RealSenseInterface>(width,height,fps);
#elif defined WITH_OPENNI2
  cam = std::make_shared<OpenNI2Interface>(flipRows,width,height,fps);
#else
  std::cerr << "ERROR: Compiled without any camera support.\n";
  cam = nullptr;
  return EXIT_FAILURE;
#endif

  logger = std::make_unique<Logger>(outDir,cam);

  glutInit(&argc,argv);

  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE); // | GLUT_DEPTH
  glutInitWindowSize(width*2,height);

  glutCreateWindow("LoggerRealSense");

  glutKeyboardFunc(keyboardCallback);
  glutDisplayFunc(displayCallback);
  glutIdleFunc(idleCallback);

  glutMainLoop();
}