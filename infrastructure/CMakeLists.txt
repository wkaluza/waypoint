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
  ${PROJECT_ROOT_DIR}/src/waypoint/smart_ptrs.cpp
  PUBLIC_HEADERS
  ${PROJECT_ROOT_DIR}/src/waypoint/include/waypoint/waypoint.hpp
  PRIVATE_HEADERS
  ${PROJECT_ROOT_DIR}/src/waypoint/internal/types.hpp
  ${PROJECT_ROOT_DIR}/src/waypoint/internal/impls.hpp)

new_basic_test(000_include_only)
new_basic_test(001_init_only)
new_basic_test(002_empty_autorun_block_with_init)
new_basic_test(003_two_empty_autoruns_with_init)
new_basic_test(004_empty_autorun_block_no_init)
new_basic_test(005_two_empty_autoruns_no_init)
new_basic_test(006_one_empty_group)
new_basic_test(007_two_empty_groups_diff_names)
new_basic_test(008_two_empty_groups_same_name)
new_basic_test(009_trivial_test)
new_basic_test(010_two_tests_same_group_diff_name)
new_basic_test(011_two_tests_diff_group_diff_name)
new_basic_test(012_two_tests_diff_group_same_name)
new_basic_test(013_two_tests_same_group_same_name)
new_basic_test(014_passing_test)
new_basic_test(015_failing_test)
new_basic_test(016_passing_test_with_auto)
new_basic_test(017_failing_test_with_auto)
new_basic_test(018_failing_test_with_auto)
new_basic_test(019_failing_test_with_auto)
new_basic_test(020_failing_test_with_auto)
new_basic_test(021_failing_test_with_auto)
new_basic_test(022_failing_test_with_auto)
new_basic_test(023_failing_test_with_auto)
new_basic_test(024_failing_test_with_auto)
new_basic_test(025_failing_test_with_auto)
new_basic_test(026_multiple_tests)

new_basic_test(027_internal_smart_ptrs)
target_include_directories(
  027_internal_smart_ptrs
  PRIVATE ${PROJECT_ROOT_DIR}/src/waypoint/internal
          ${PROJECT_ROOT_DIR}/src/waypoint/include/waypoint)

new_basic_test(028_assertion_messages)
new_basic_test(029_results)
new_basic_test(030_capturing_lambda_as_test_body)
new_basic_test(031_mutable_lambda_as_test_body)
new_basic_test(032_setup)
new_basic_test(033_teardown)

new_basic_test(034_internal_function)
target_include_directories(
  034_internal_function
  PRIVATE ${PROJECT_ROOT_DIR}/src/waypoint/internal
          ${PROJECT_ROOT_DIR}/src/waypoint/include/waypoint)
