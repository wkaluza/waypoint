# Forthright unit testing

The Waypoint project is a minimalistic unit testing framework written
in modern C++.
It aims to be stable, intuitive, and quick to master.
Below are some of Waypoint's stand-out features.

* Simple API, only one header
* Only one macro (used for automatic test registration)
* Tests are defined within normal program flow
* Crash resilience (a crashing test does not interfere
  with other tests)
* Uses modern C++23 features internally, but can be adopted even in
  C++11 codebases
* Tests are executed sequentially and shuffled by default

## Quick start

It is easiest to integrate Waypoint into your project if you use
[CMake](https://cmake.org).
You will need to install [Ninja](https://ninja-build.org) to use the
Ninja Multi-Config generator.

Building Waypoint requires a C++23-capable compiler.
It has been confirmed to work out of the box with GCC 15 and Clang 20.
We will use the latter in the following examples.

### The build-and-install method (recommended)

Start by cloning the Waypoint Git repository and navigate to the
clone's directory.

```shell
git clone https://github.com/wkaluza/waypoint.git
cd waypoint
```

To build Waypoint and install the artifacts to a specified location,
execute the following commands.

```shell
CC=clang-20 CXX=clang++-20 cmake -S infrastructure -B build___ -G "Ninja Multi-Config"

cmake --build build___ --config Debug
cmake --build build___ --config RelWithDebInfo
cmake --build build___ --config Release

cmake --install build___ --prefix waypoint_install___ --config Debug
cmake --install build___ --prefix waypoint_install___ --config RelWithDebInfo
cmake --install build___ --prefix waypoint_install___ --config Release
```

If all went well, the directory `waypoint_install___` now exists.
You are free to rename it if you like.
Copy this directory into your test project's directory, possibly adding
it to your `.gitignore` file. 

Below is a minimal C++ CMake project which makes use of the build
artifacts.
All files are assumed to be in the same directory together with
`waypoint_install___`.

`the_answer.hpp`

```c++
#pragma once

namespace stuff {

int the_answer() noexcept;

}
```

`the_answer.cpp`

```c++
#include "the_answer.hpp"

namespace stuff {

int the_answer() noexcept
{
  return 42;
}

}
```

`CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 4.0)

project(
  test_project
  VERSION 1.0.0
  DESCRIPTION "Test project"
  LANGUAGES CXX)

enable_testing()

find_package(
  waypoint
  REQUIRED
  CONFIG
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/waypoint_install___/cmake
  NO_DEFAULT_PATH)

add_library(the_answer)
target_sources(the_answer
  PRIVATE the_answer.cpp
  PUBLIC
  FILE_SET the_answer_public_headers
  TYPE HEADERS
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
  FILES the_answer.hpp)
target_compile_features(the_answer PRIVATE cxx_std_11)

# TODO: add test executable
```
