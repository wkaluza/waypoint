cmake_minimum_required(VERSION 4.0)

project(
  project_waypoint_find_package_exact_version_test
  VERSION 1.0.0
  DESCRIPTION
    "Test project for loading waypoint with find_package (exact version)"
  LANGUAGES CXX)

enable_testing()

include(${CMAKE_CURRENT_SOURCE_DIR}/tools.cmake)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  find_package(
    waypoint
    0.0.1
    EXACT
    REQUIRED
    CONFIG
    PATHS
    ${PROJECT_ROOT_DIR}/waypoint_install_linux_clang___/cmake
    NO_DEFAULT_PATH)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  find_package(
    waypoint
    0.0.1
    EXACT
    REQUIRED
    CONFIG
    PATHS
    ${PROJECT_ROOT_DIR}/waypoint_install_linux_gcc___/cmake
    NO_DEFAULT_PATH)
else()
  message(FATAL_ERROR "Unexpected compiler")
endif()

add_custom_target(all_tests)

new_target(
  STATIC
  TARGET
  stub_library
  DIRECTORY
  src/stub_library
  SOURCES
  stub_library.cpp
  PUBLIC_HEADERS
  stub_library.hpp)

new_target(
  TEST
  TARGET
  find_package_test
  DIRECTORY
  test/find_package_test
  SOURCES
  main.cpp
  LINKS
  stub_library
  waypoint::waypoint)
