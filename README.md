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

It is easiest to integrate Waypoint into your project if you use CMake.

TODO: describe Waypoint build and installation process

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
  PATHS /path/to/waypoint_install_dir/cmake
  NO_DEFAULT_PATH)
```
