# Lucid unit testing with Waypoint

## Contents

1. [Introduction](#introduction)
2. [Quick start](#quick-start)
    1. [The build-and-install method (recommended)](#the-build-and-install-method-recommended)
    2. [The add_subdirectory method](#the-add_subdirectory-method)
3. [Contributing](#contributing)

## Introduction

Waypoint is a minimalistic unit testing framework written in modern
C++.
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
You are free to rename it if you wish.
Copy this directory into your test project's directory, possibly adding
it to your `.gitignore` file.

In the `examples/quick_start1` directory of this repository, there is
a minimal C++ CMake test project which makes use of the build artifacts in
`waypoint_install___`.

TODO: describe how to build and run test project

### The add_subdirectory method

TODO

### Providing your own entry point

TODO

## Contributing

TODO
