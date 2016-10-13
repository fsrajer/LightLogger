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
vector<uint16_t> depthBuffer;
vector<uint8_t> rgbBuffer;

void displayCallback()
{
  int lastDepth = cam->latestDepthIndex;
  if(lastDepth == -1)
  {
    return;
  }
  int bufferIdx = lastDepth % CameraInterface::numBuffers;
  memcpy(depthBuffer.data(),cam->frameBuffers[bufferIdx].first.first,WIDTH*HEIGHT*sizeof(uint16_t));
  memcpy(rgbBuffer.data(),cam->frameBuffers[bufferIdx].first.second,WIDTH*HEIGHT*3*sizeof(uint8_t));

  glClear(GL_COLOR_BUFFER_BIT);
  glPixelZoom(1,-1);

  // Display depth data by linearly mapping depth between 0 and 2 meters to the red channel
  glRasterPos2f(-1,1);
  glPixelTransferf(GL_RED_SCALE,0xFFFF * cam->depthScale() / 2.0f);
  glDrawPixels(WIDTH,HEIGHT,GL_RED,GL_UNSIGNED_SHORT,depthBuffer.data());
  glPixelTransferf(GL_RED_SCALE,1.0f);

  // Display color image as RGB triples
  glRasterPos2f(0,1);
  glDrawPixels(WIDTH,HEIGHT,GL_RGB,GL_UNSIGNED_BYTE,rgbBuffer.data());

  glutSwapBuffers();
}
void idleCallback()
{
  glutPostRedisplay();
}

int main(int argc,char *argv[])
{
  cam = std::make_unique<RealSenseInterface>(WIDTH,HEIGHT,FPS);
  depthBuffer.resize(WIDTH*HEIGHT);
  rgbBuffer.resize(WIDTH*HEIGHT*3);

  glutInit(&argc,argv);

  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE); // | GLUT_DEPTH
  glutInitWindowSize(WIDTH*2,HEIGHT);

  glutCreateWindow("LoggerRealSense");

  //glutKeyboardFunc(keyboardCallback);
  glutDisplayFunc(displayCallback);
  glutIdleFunc(idleCallback);
  

  glutMainLoop();
}