#include <iostream>
#include <memory>
#include <vector>
#include <cstdint>

#include "GL/glut.h"
#include "librealsense/rs.hpp"

#include "CameraInterface.h"
#include "RealSenseInterface.h"

using std::cout;
using std::vector;

#define WIDTH 640
#define HEIGHT 480
#define FPS 30

std::unique_ptr<CameraInterface> cam;

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
  glDrawPixels(WIDTH,HEIGHT,GL_RED,GL_UNSIGNED_SHORT,depthData);
  glPixelTransferf(GL_RED_SCALE,1.0f);

  // Display color image as RGB triples
  glRasterPos2f(0,1);
  glDrawPixels(WIDTH,HEIGHT,GL_RGB,GL_UNSIGNED_BYTE,rgbData);

  glutSwapBuffers();
}
void idleCallback()
{
  glutPostRedisplay();
}

int main(int argc,char *argv[])
{
  cam = std::make_unique<RealSenseInterface>(WIDTH,HEIGHT,FPS);

  glutInit(&argc,argv);

  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE); // | GLUT_DEPTH
  glutInitWindowSize(WIDTH*2,HEIGHT);

  glutCreateWindow("LoggerRealSense");

  //glutKeyboardFunc(keyboardCallback);
  glutDisplayFunc(displayCallback);
  glutIdleFunc(idleCallback);
  

  glutMainLoop();
}