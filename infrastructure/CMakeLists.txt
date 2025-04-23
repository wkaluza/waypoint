cmake_minimum_required(VERSION 4.0)

project(waypoint LANGUAGES CXX)

include(tools.cmake)

enable_testing()

set(PROJECT_ROOT_DIR ${CMAKE_SOURCE_DIR}/..)

new_target(
  STATIC
  TARGET
  waypoint
  DIRECTORY
  ${PROJECT_ROOT_DIR}/src/waypoint
  SOURCES
  ${PROJECT_ROOT_DIR}/src/waypoint/waypoint.cpp
  INTERFACE_HEADERS
  ${PROJECT_ROOT_DIR}/src/waypoint/include/waypoint/waypoint.hpp)

new_target(
  TEST
  TARGET
  test_000_include_only
  SOURCES
  ${PROJECT_ROOT_DIR}/test/000_include_only/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  test_001_init_only
  SOURCES
  ${PROJECT_ROOT_DIR}/test/001_init_only/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  test_002_empty_autorun_block_with_init
  SOURCES
  ${PROJECT_ROOT_DIR}/test/002_empty_autorun_block_with_init/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  test_003_two_empty_autorun_blocks_with_init
  SOURCES
  ${PROJECT_ROOT_DIR}/test/003_two_empty_autorun_blocks_with_init/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  test_004_empty_autorun_block_without_init
  SOURCES
  ${PROJECT_ROOT_DIR}/test/004_empty_autorun_block_without_init/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  test_005_two_empty_autorun_blocks_without_init
  SOURCES
  ${PROJECT_ROOT_DIR}/test/005_two_empty_autorun_blocks_without_init/main.cpp
  LINKS
  waypoint)
