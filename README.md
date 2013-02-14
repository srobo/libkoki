# libkoki

**libkoki** is an open source computer vision library, written in C, for finding and estimating the position of libkoki markers in an image.
The library was developed for use by [Student Robotics](https://www.studentrobotics.org) competitors, but was designed to be as generally useful as possible.
Unlike ARToolkit's (a toolkit for Augmented Reality), libkoki uses a 36-bit code grid with error detection and correction to identify different markers.
This removes the false-positive identification issue commonly experienced with ARToolkit's pattern matching system.

## Features

 * 200+ unique marker codes available for use
 * relative world position estimation, given a marker of known size
 * Uses OpenCV's IplImage, a common format
 * V4L2 helper functions for streaming from webcams
 * Error correction using Hamming(7,4) codes, and error detection using CRC12
 * Operates on a frame-by-frame basis, so both video and still images can be used
 * Python bindings


## Source Code

The canonical sources for libkoki and related projects are:

 * **libkoki**, the main library: [`git://srobo.org/libkoki.git`](git://srobo.org/libkoki.git)
 * **pykoki**, the Python bindings for libkoki: [`git://srobo.org/pykoki.git`](git://srobo.org/pykoki.git)
 * **kokimarker**, the library for generating markers: [`git://srobo.org/kokimarker.git`](git://srobo.org/kokimarker.git)


## Build Requirements

In order to build libkoki you'll need to have:

 * A C compiler (with GNU99 support, e.g. GCC)
 * OpenCV headers
 * YAML headers
 * OpenGL Utility Toolkit
 * SCons (a software build tool)
 * Doxygen (for generating documentation)

On Ubuntu 11.10 and 11.04, these can be installed using:

~~~~~~~~~~~~~~~~
(sudo) $ apt-get install gcc libcv-dev libhighgui-dev libcvaux-dev libyaml-dev freeglut3-dev doxygen
~~~~~~~~~~~~~~~~


## Build Instructions

To build libkoki, run `scons` in the root source directory.

## Examples

libkoki contains a number of examples programs that help demonstrate how
to use the API, as well as some of its capabilities.

The examples are built by default as part of the normal build process.

In order to run the examples (without installing libkoki) you should run
the `shell` script in the root of the repository, and then cd into the
examples directory:

~~~~~~~~~~~~~~~~
$ ./shell
$ cd examples
~~~~~~~~~~~~~~~~

From there you can run any of examples by runnning them as local programs.
For instance:

~~~~~~~~~~~~~~~~
$ ./realtime_gl
~~~~~~~~~~~~~~~~

Note that some of them expect to be passed an image file as their first
(and only) argument:

~~~~~~~~~~~~~~~~
$ ./marker_info image_from_camera.jpg
~~~~~~~~~~~~~~~~

If get an error that looks a bit like:

~~~~~~~~~~~~~~~~
$ ./examples/realtime_quads
./examples/realtime_quads: error while loading shared libraries: libkoki.so: cannot open shared object file: No such file or directory
~~~~~~~~~~~~~~~~

Then most likely you've forgotten to run `./shell`.
