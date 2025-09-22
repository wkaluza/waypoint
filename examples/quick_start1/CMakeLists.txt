cmake_minimum_required(VERSION 4.0)

project(
  test_project
  VERSION 1.0.0
  DESCRIPTION "Test project"
  LANGUAGES CXX)

enable_testing()

find_package(
  waypoint REQUIRED CONFIG PATHS
  ${CMAKE_CURRENT_SOURCE_DIR}/waypoint_install___/cmake NO_DEFAULT_PATH)

add_library(the_answer)
target_sources(
  the_answer
  PRIVATE the_answer.cpp
  PUBLIC FILE_SET
         the_answer_public_headers
         TYPE
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_SOURCE_DIR}
         FILES
         the_answer.hpp)
target_compile_features(the_answer PRIVATE cxx_std_11)

# TODO: add test executable
