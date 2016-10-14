# LoggerRealSense #

Lightweigth program for saving depth and color streams from Intel RealSense camera into .klg format for [ElasticFusion](https://github.com/mp3guy/ElasticFusion). 
Inspired by [Logger2] (https://github.com/mp3guy/Logger2)

## Controls ##

```bash
LoggerRealSense.exe c:/path/to/output/dir
```

Press button R to start recording and press it again to stop. 
Stream is saved into the output directory in a file seq0. You can record multiple videos; the seq id will be automatically increased.

## Dependencies ##

* librealsense
* freeglut
* zlib (optional but strongly recommended)
* libjpeg (optional but strongly recommended)

## License ##

This software is under the BSD 3-Clause License. See LICENSE.txt.