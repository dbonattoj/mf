Media Framework
===============

C++ framework for video and 3D processing.

Media flow graph system with support for inter-node parallelism, past and future time windows multiple inputs,
seekable or real-time streams, and more. Minimal high-level interface for concrete node classes which do image
processing.

Core components for image processing, camera models, point clouds. Generalized to ndarray data type where elements
may be vectors, tuples or nullable. Support for interfacing with OpenCV, or other external code.

Developped in modern C++14. Extensively tested. Some low-level OS-specific components.

### Requirements:
  * C++14 compatible compiler (Clang tested)
  * OpenCV
  * Eigen
  * Linux or Darwin currently



