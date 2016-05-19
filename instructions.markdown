---
title: mf - Build and usage guide
---

The package consists of the framework, its unit tests, and a demonstration program which implements an FTV view
synthesis algorithm. The project website is <http://timlenertz.github.io/mf/>, the current version of the repository
is at <https://github.com/timlenertz/mf>.

## Requirements
The framework requires Linux or Darwin (OS X). Some small bugs currently remain in the Darwin implementation which can result
in locking, the Linux version works best.

A compiler with full support for C++14 is needed, and the framework was developped and tested using _LLVM/Clang_ 3.8.

The libraries _OpenCV_ <http://opencv.org/>  and _Eigen_ <http://eigen.tuxfamily.org/> need to be installed on the
system. They are available as free downloads, and also as `deb` or `rpm`. packages in Linux package managers.


## Compilation
The single Makefile for building the framework, unit tests and demonstration program is located in the top level project
directory. The first line of `Makefile.linux` or `Makefile.darwin` may need to be adjusted to the name of the compiler
on the system. (typically `clang++`)

To build the library and demonstration program, run:

    make

The framework itself gets compiled into a shared library and installed in `dist/libmf.so`. (or `dist/libmf.dylib` on
Darwin). The executable of the demonstration program gets installed in `dist/prog/view_synthesis`. For the purpose of
illustration, the filters that it uses also get compiled into shared libraries, installed in `dist/prog/filter/*.so`.

To build and run the unit tests, run:

    make test

The unit test verify about 2000 assertions and display "All tests passed" when completed successfully.


## Source code documentation
Documentation of the source code can be generated using _Doxygen_ <http://www.stack.nl/~dimitri/doxygen/>. To generate,
run

    doxygen

The HTML documentation is outputed in `doc/doxy/index.html`.


## Demonstration program
A scaled version of the Poznan Sequence is available on the project website. To run the demonstration program, it needs
to be unpacked to `poznan_blocks/` in the project directory.

The program is run with

    dist/prog/view_synthesis output.avi

It outputs the video to `output.avi`.

The source code of the program is located in `prog/view_synthesis.cc`, and that of its filter node in `prog/filters/`. 
