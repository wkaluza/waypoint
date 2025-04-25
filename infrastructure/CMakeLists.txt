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
  ${PROJECT_ROOT_DIR}/src/waypoint/string.cpp
  ${PROJECT_ROOT_DIR}/src/waypoint/test_types.cpp
  ${PROJECT_ROOT_DIR}/src/waypoint/vector.cpp
  PUBLIC_HEADERS
  ${PROJECT_ROOT_DIR}/src/waypoint/include/waypoint/waypoint.hpp
  ${PROJECT_ROOT_DIR}/src/waypoint/include/waypoint/string.hpp
  ${PROJECT_ROOT_DIR}/src/waypoint/include/waypoint/test_types.hpp
  ${PROJECT_ROOT_DIR}/src/waypoint/include/waypoint/vector.hpp)

new_target(
  TEST
  TARGET
  000_include_only
  SOURCES
  ${PROJECT_ROOT_DIR}/test/000_include_only/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  001_init_only
  SOURCES
  ${PROJECT_ROOT_DIR}/test/001_init_only/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  002_empty_autorun_block_with_init
  SOURCES
  ${PROJECT_ROOT_DIR}/test/002_empty_autorun_block_with_init/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  003_two_empty_autorun_blocks_with_init
  SOURCES
  ${PROJECT_ROOT_DIR}/test/003_two_empty_autorun_blocks_with_init/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  004_empty_autorun_block_without_init
  SOURCES
  ${PROJECT_ROOT_DIR}/test/004_empty_autorun_block_without_init/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  005_two_empty_autorun_blocks_without_init
  SOURCES
  ${PROJECT_ROOT_DIR}/test/005_two_empty_autorun_blocks_without_init/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  006_one_empty_group
  SOURCES
  ${PROJECT_ROOT_DIR}/test/006_one_empty_group/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  007_two_empty_groups_different_names
  SOURCES
  ${PROJECT_ROOT_DIR}/test/007_two_empty_groups_different_names/main.cpp
  LINKS
  waypoint)
