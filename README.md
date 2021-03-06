# mf Media Framework

## Being moved to https://github.com/timlenertz/tff

C++ framework for multi-stream video and 3D processing.

Flow graph system with support for inter-node parallelism, past and future time windows on multiple inputs,
seekable or real-time streams, and more. Minimal high-level interface for concrete filter classes which do image
processing.

Core components and media toolkit for image processing, camera models, point clouds. Generalized to n-d array
data type where elements may be vectors, tuples or null-able. Support for interfacing with OpenCV, or other external code.

Developped in modern C++14. Extensively tested. Some low-level OS-specific components.

### Requirements:
  * C++14 compatible compiler (tested on Clang/LLVM)
  * OpenCV (http://opencv.org/)
  * Eigen (http://eigen.tuxfamily.org/)
  * Linux or Darwin OS currently

