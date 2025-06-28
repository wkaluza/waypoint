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
  ${PROJECT_ROOT_DIR}/src/waypoint/core_actions.cpp
  ${PROJECT_ROOT_DIR}/src/waypoint/get_impl.cpp
  ${PROJECT_ROOT_DIR}/src/waypoint/impls.cpp
  PUBLIC_HEADERS
  ${PROJECT_ROOT_DIR}/src/waypoint/include/waypoint/waypoint.hpp
  PRIVATE_HEADERS
  ${PROJECT_ROOT_DIR}/src/waypoint/internal/types.hpp
  ${PROJECT_ROOT_DIR}/src/waypoint/internal/impls.hpp)

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
  003_two_empty_autoruns_with_init
  SOURCES
  ${PROJECT_ROOT_DIR}/test/003_two_empty_autoruns_with_init/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  004_empty_autorun_block_no_init
  SOURCES
  ${PROJECT_ROOT_DIR}/test/004_empty_autorun_block_no_init/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  005_two_empty_autoruns_no_init
  SOURCES
  ${PROJECT_ROOT_DIR}/test/005_two_empty_autoruns_no_init/main.cpp
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
  007_two_empty_groups_diff_names
  SOURCES
  ${PROJECT_ROOT_DIR}/test/007_two_empty_groups_diff_names/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  008_two_empty_groups_same_name
  SOURCES
  ${PROJECT_ROOT_DIR}/test/008_two_empty_groups_same_name/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  009_trivial_test
  SOURCES
  ${PROJECT_ROOT_DIR}/test/009_trivial_test/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  010_two_tests_same_group_diff_name
  SOURCES
  ${PROJECT_ROOT_DIR}/test/010_two_tests_same_group_diff_name/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  011_two_tests_diff_group_diff_name
  SOURCES
  ${PROJECT_ROOT_DIR}/test/011_two_tests_diff_group_diff_name/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  012_two_tests_diff_group_same_name
  SOURCES
  ${PROJECT_ROOT_DIR}/test/012_two_tests_diff_group_same_name/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  013_two_tests_same_group_same_name
  SOURCES
  ${PROJECT_ROOT_DIR}/test/013_two_tests_same_group_same_name/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  014_passing_test
  SOURCES
  ${PROJECT_ROOT_DIR}/test/014_passing_test/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  015_failing_test
  SOURCES
  ${PROJECT_ROOT_DIR}/test/015_failing_test/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  016_passing_test_with_auto
  SOURCES
  ${PROJECT_ROOT_DIR}/test/016_passing_test_with_auto/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  017_failing_test_with_auto
  SOURCES
  ${PROJECT_ROOT_DIR}/test/017_failing_test_with_auto/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  018_failing_test_with_auto
  SOURCES
  ${PROJECT_ROOT_DIR}/test/018_failing_test_with_auto/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  019_failing_test_with_auto
  SOURCES
  ${PROJECT_ROOT_DIR}/test/019_failing_test_with_auto/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  020_failing_test_with_auto
  SOURCES
  ${PROJECT_ROOT_DIR}/test/020_failing_test_with_auto/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  021_failing_test_with_auto
  SOURCES
  ${PROJECT_ROOT_DIR}/test/021_failing_test_with_auto/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  022_failing_test_with_auto
  SOURCES
  ${PROJECT_ROOT_DIR}/test/022_failing_test_with_auto/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  023_failing_test_with_auto
  SOURCES
  ${PROJECT_ROOT_DIR}/test/023_failing_test_with_auto/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  024_failing_test_with_auto
  SOURCES
  ${PROJECT_ROOT_DIR}/test/024_failing_test_with_auto/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  025_failing_test_with_auto
  SOURCES
  ${PROJECT_ROOT_DIR}/test/025_failing_test_with_auto/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  026_multiple_tests
  SOURCES
  ${PROJECT_ROOT_DIR}/test/026_multiple_tests/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  027_unique_ptr
  SOURCES
  ${PROJECT_ROOT_DIR}/test/027_unique_ptr/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  028_assertion_messages
  SOURCES
  ${PROJECT_ROOT_DIR}/test/028_assertion_messages/main.cpp
  LINKS
  waypoint)

new_target(
  TEST
  TARGET
  029_results
  SOURCES
  ${PROJECT_ROOT_DIR}/test/029_results/main.cpp
  LINKS
  waypoint)
