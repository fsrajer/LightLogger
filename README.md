# LightLogger #

Lightweigth program for saving depth and color streams from Intel RealSense camera and OpenNI2-supported cameras into .klg format for [ElasticFusion](https://github.com/mp3guy/ElasticFusion). 
Inspired by [Logger2] (https://github.com/mp3guy/Logger2)

## Controls ##

```bash
LightLogger.exe c:/path/to/output/dir
```

Press button R to start recording and press it again to stop. 
Stream is saved into the output directory in a file seq0. You can record multiple videos; the seq id will be automatically increased.

(OpenNI2 is by default mirrroring rows of OpenNI2 cameras because my camera was giving me mirrored images. You can switch this off in main.cpp by flipRows variable)

## Dependencies ##

* [librealsense](https://github.com/IntelRealSense/librealsense) (optional)
* [OpenNI2] (http://structure.io/openni) (optional)
* [freeglut](http://freeglut.sourceforge.net/)
* [zlib](https://github.com/madler/zlib.git) (optional but strongly recommended)
* [libjpeg](https://github.com/LuaDist/libjpeg.git) (optional but strongly recommended)

## License ##

This software is under the BSD 3-Clause License. See LICENSE.txt.
