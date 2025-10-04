# Lucid C++ unit testing with Waypoint

## Contents

1. [License](#license)
2. [Introduction](#introduction)
3. [Quick start](#quick-start)
    1. [The build-and-install method (recommended)](#the-build-and-install-method-recommended)
    2. [The add_subdirectory method](#the-add_subdirectory-method)
    3. [Providing your own entry point](#providing-your-own-entry-point)
4. [Releases](#releases)
5. [Contributing to Waypoint](#contributing-to-waypoint)

## License

Waypoint is available under the MIT license.
For more information, see the [LICENSE](../LICENSE) file in the root directory
of this project.

## Introduction

Waypoint is a minimalistic unit testing framework written in modern C++.
It aims to be stable, intuitive, and quick to master.
Below are some of Waypoint's stand-out features.

* Simple API, only one header
* Only one macro (used for automatic test registration)
* Tests are defined within normal program flow
* Crash resilience (a crashing test does not interfere
  with other tests)
* Uses modern C++23 features internally, but can be adopted even in C++11
  codebases
* Tests are executed in shuffled order by default

## Quick start

It is easiest to integrate Waypoint into your project if you use
[CMake](https://cmake.org).
You will need to install [Ninja](https://ninja-build.org) to use the
Ninja Multi-Config generator.

Building Waypoint requires a C++23-capable compiler.
It has been confirmed to work out of the box with GCC 15 and Clang 20.
We will use the latter in the examples that follow.

### The build-and-install method (recommended)

Start by cloning the Waypoint Git repository and navigate to the
clone's directory.

```shell
git clone ssh://git@github.com/wkaluza/waypoint.git
cd waypoint
```

To build Waypoint and install the artifacts to a specified location,
execute the following commands.

```shell
cd infrastructure
# Configure step
# You may use -DBUILD_SHARED_LIBS=TRUE if you wish
# to produce a dynamic library.
CC=clang-20 CXX=clang++-20 cmake --preset example_configure 

# Build step
cmake --build --preset example_build --target all --config Debug
cmake --build --preset example_build --target all --config RelWithDebInfo
cmake --build --preset example_build --target all --config Release

# Install step
cmake --build --preset example_build --target install --config Debug
cmake --build --preset example_build --target install --config RelWithDebInfo
cmake --build --preset example_build --target install --config Release
```

If all went well, the directory `waypoint_install___` now exists.
You are free to rename it if you wish, but for the purposes of this
example, let us keep the name as it is.

In the `examples/quick_start_build_and_install` directory of this
repository, there is a minimal C++ CMake test project which makes use
of the artifacts in `waypoint_install___`.

To build and run the test project, start by copying
`waypoint_install___` into `examples/quick_start_build_and_install`.
In a production scenario, you would probably also add it to your
`.gitignore` file.

```shell
cd examples/quick_start_build_and_install
cp --recursive ../../waypoint_install___ ./

# Configure step
cmake --preset example_configure

# Build step
cmake --build --preset example_build --config Debug

# Run the tests with CMake
cmake --build --preset example_build --target test --config Debug

# Alternatively, run the tests with CTest (a more flexible approach)
ctest --preset example_test --build-config Debug

# You may also run the test executable directly
./build___/Debug/test_program
```

### The add_subdirectory method

TODO

### Providing your own entry point

TODO

## Releases

The Waypoint project follows the Live at Head philosophy.
Every commit undergoes verification to be implicitly releasable and
backwards compatible.
As such, there is no formal feature-driven release schedule and we
encourage consumers to use the latest commit (the "Head") of the `main`
branch.

We will occasionally (e.g. on an annual basis) tag a `main` branch head
commit with a [Semantic Version](https://semver.org) number to mark it
as "supported".
This is in deference to users who require numbered dependency versions,
e.g. due to internal company policies, or who rely on GitHub's Releases
facility.
Tagging also gives us an opportunity to mark milestones, such as the first
feature-complete `1.0.0` version.

If a sufficiently serious defect is discovered in a supported version,
a branch may be created at the supported commit; the fix will be
backported to this branch from `main`.
The commit implementing the fix will be tagged as a patch release.
Supported versions will receive bugfixes for at least two years from
their base supported commit's creation date.

If a client-facing API requires changes that break backwards
compatibility, a deprecation period of at least one year will be
observed prior to removal, unless exceptional circumstances arise
(e.g. a security exploit would unavoidably put users at risk).

## Contributing to Waypoint

Refer to the [contribution guidelines](CONTRIBUTING.md).
