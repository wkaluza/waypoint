include_guard(GLOBAL)

set(PROJECT_ROOT_DIR ${CMAKE_SOURCE_DIR}/..)

set(WAYPOINT_INTERNAL_HEADER_DIR ${PROJECT_ROOT_DIR}/src/waypoint/internal)
set(WAYPOINT_OWN_HEADER_DIR ${PROJECT_ROOT_DIR}/src/waypoint/include/waypoint)

set(MAIN_LIBRARY waypoint)
set(INTERNAL_LIBRARIES assert autorun coverage process)

function(new_target_)
  set(options EXECUTABLE STATIC TEST ENABLE_EXCEPTIONS_IN_COVERAGE
              EXCLUDE_FROM_ALL)
  set(singleValueKeywords DIRECTORY TARGET)
  set(multiValueKeywords LINKS PRIVATE_HEADERS PUBLIC_HEADERS SOURCES)
  cmake_parse_arguments(PARSE_ARGV 0 "arg" "${options}"
                        "${singleValueKeywords}" "${multiValueKeywords}")

  if(arg_EXECUTABLE)
    add_executable(${arg_TARGET})
  endif()
  if(arg_TEST)
    add_executable(${arg_TARGET})
    set_target_properties(${arg_TARGET} PROPERTIES EXCLUDE_FROM_ALL TRUE)

    if(TARGET test_helpers)
      target_link_libraries(${arg_TARGET} PRIVATE test_helpers)
    endif()
    if(TARGET all_tests)
      add_dependencies(all_tests ${arg_TARGET})
    endif()

    add_test(NAME test_${arg_TARGET} COMMAND $<TARGET_FILE:${arg_TARGET}>)
    set_tests_properties(test_${arg_TARGET} PROPERTIES LABELS test)
    set_tests_properties(
      test_${arg_TARGET}
      PROPERTIES ENVIRONMENT
                 WAYPOINT_INTERNAL_RUNNING_TEST_XTSyiOp7QMFW8P2H=123)

    add_test(
      NAME valgrind_${arg_TARGET}
      COMMAND valgrind --leak-check=yes --error-exitcode=1
              $<TARGET_FILE:${arg_TARGET}>
      CONFIGURATIONS Debug)
    set_tests_properties(valgrind_${arg_TARGET} PROPERTIES LABELS valgrind)
    set_tests_properties(
      valgrind_${arg_TARGET}
      PROPERTIES ENVIRONMENT
                 WAYPOINT_INTERNAL_RUNNING_TEST_XTSyiOp7QMFW8P2H=123)
  endif()
  if(arg_STATIC)
    add_library(${arg_TARGET} STATIC)
  endif()

  if(arg_EXCLUDE_FROM_ALL)
    set_target_properties(${arg_TARGET} PROPERTIES EXCLUDE_FROM_ALL TRUE)
  endif()

  target_compile_features(
    ${arg_TARGET}
    PRIVATE cxx_std_23
    PUBLIC cxx_std_17)

  if(DEFINED PRESET_ENABLE_COVERAGE)
    target_compile_options(${arg_TARGET} PRIVATE ${PRESET_ENABLE_COVERAGE})
    target_link_options(${arg_TARGET} PRIVATE ${PRESET_ENABLE_COVERAGE})

    if(NOT arg_ENABLE_EXCEPTIONS_IN_COVERAGE)
      target_compile_options(${arg_TARGET} PRIVATE -fno-exceptions)
    endif()

    target_compile_definitions(
      ${arg_TARGET} PRIVATE WAYPOINT_INTERNAL_COVERAGE_IOm5lSCCB6p0j19)
  endif()

  if(DEFINED arg_SOURCES)
    target_sources(${arg_TARGET} PRIVATE ${arg_SOURCES})
  endif()

  if(DEFINED arg_LINKS)
    target_link_libraries(${arg_TARGET} PRIVATE ${arg_LINKS})
  endif()

  if(DEFINED arg_DIRECTORY)
    if(DEFINED arg_PRIVATE_HEADERS)
      target_sources(
        ${arg_TARGET}
        PRIVATE FILE_SET
                internal_headers_${arg_TARGET}
                TYPE
                HEADERS
                BASE_DIRS
                ${arg_DIRECTORY}/internal
                FILES
                ${arg_PRIVATE_HEADERS})
    endif()

    if(DEFINED arg_PUBLIC_HEADERS)
      target_sources(
        ${arg_TARGET}
        PRIVATE FILE_SET
                own_headers_${arg_TARGET}
                TYPE
                HEADERS
                BASE_DIRS
                ${arg_DIRECTORY}/include/${arg_TARGET}
                FILES
                ${arg_PUBLIC_HEADERS})

      target_sources(
        ${arg_TARGET}
        INTERFACE FILE_SET
                  interface_headers_${arg_TARGET}
                  TYPE
                  HEADERS
                  BASE_DIRS
                  ${arg_DIRECTORY}/include
                  FILES
                  ${arg_PUBLIC_HEADERS})
    endif()
  endif()

  if(DEFINED PRESET_ELEVATED_COMPILER_WARNINGS)
    separate_arguments(PRESET_ELEVATED_COMPILER_WARNINGS)
    target_compile_options(${arg_TARGET}
                           PRIVATE ${PRESET_ELEVATED_COMPILER_WARNINGS})
  endif()

  if(DEFINED PRESET_PROP_COMPILE_WARNING_AS_ERROR)
    set_target_properties(
      ${arg_TARGET} PROPERTIES COMPILE_WARNING_AS_ERROR
                               ${PRESET_PROP_COMPILE_WARNING_AS_ERROR})
  endif()

  if(DEFINED PRESET_PROP_LINK_WARNING_AS_ERROR)
    set_target_properties(
      ${arg_TARGET} PROPERTIES LINK_WARNING_AS_ERROR
                               ${PRESET_PROP_LINK_WARNING_AS_ERROR})
  endif()

  if(DEFINED PRESET_PROP_CXX_EXTENSIONS)
    set_target_properties(
      ${arg_TARGET} PROPERTIES CXX_EXTENSIONS ${PRESET_PROP_CXX_EXTENSIONS})
  endif()

  if(DEFINED PRESET_PROP_EXPORT_COMPILE_COMMANDS)
    set_target_properties(
      ${arg_TARGET} PROPERTIES EXPORT_COMPILE_COMMANDS
                               ${PRESET_PROP_EXPORT_COMPILE_COMMANDS})
  endif()
endfunction()

function(new_basic_test name)
  new_target(
    TEST
    TARGET
    ${name}
    SOURCES
    test/${name}/main.cpp
    LINKS
    ${MAIN_LIBRARY})
endfunction()

function(new_impl_test name)
  new_basic_test(${name})
  target_link_libraries(${name} PRIVATE ${INTERNAL_LIBRARIES})
  target_include_directories(${name} PRIVATE ${WAYPOINT_INTERNAL_HEADER_DIR}
                                             ${WAYPOINT_OWN_HEADER_DIR})
endfunction()

function(new_target)
  set(options EXECUTABLE STATIC TEST ENABLE_EXCEPTIONS_IN_COVERAGE
              EXCLUDE_FROM_ALL)
  set(singleValueKeywords DIRECTORY TARGET)
  set(multiValueKeywords LINKS PRIVATE_HEADERS PUBLIC_HEADERS SOURCES)
  cmake_parse_arguments(PARSE_ARGV 0 "arg" "${options}"
                        "${singleValueKeywords}" "${multiValueKeywords}")

  if(arg_EXECUTABLE)
    set(type EXECUTABLE)
  endif()
  if(arg_STATIC)
    set(type STATIC)
  endif()
  if(arg_TEST)
    set(type TEST)
  endif()

  if(arg_ENABLE_EXCEPTIONS_IN_COVERAGE)
    set(exceptions ENABLE_EXCEPTIONS_IN_COVERAGE)
  else()
    set(exceptions "")
  endif()

  if(arg_EXCLUDE_FROM_ALL)
    set(exclude_from_all EXCLUDE_FROM_ALL)
  else()
    set(exclude_from_all "")
  endif()

  list(TRANSFORM arg_SOURCES PREPEND ${PROJECT_ROOT_DIR}/${arg_DIRECTORY}/)
  list(TRANSFORM arg_PRIVATE_HEADERS
       PREPEND ${PROJECT_ROOT_DIR}/${arg_DIRECTORY}/internal/)
  list(TRANSFORM arg_PUBLIC_HEADERS
       PREPEND ${PROJECT_ROOT_DIR}/${arg_DIRECTORY}/include/${arg_TARGET}/)

  new_target_(
    ${type}
    ${exceptions}
    ${exclude_from_all}
    TARGET
    ${arg_TARGET}
    DIRECTORY
    ${PROJECT_ROOT_DIR}/${arg_DIRECTORY}
    SOURCES
    ${arg_SOURCES}
    PRIVATE_HEADERS
    ${arg_PRIVATE_HEADERS}
    PUBLIC_HEADERS
    ${arg_PUBLIC_HEADERS}
    LINKS
    ${arg_LINKS})
endfunction()

function(new_platform_specific_target)
  set(options EXECUTABLE STATIC TEST ENABLE_EXCEPTIONS_IN_COVERAGE
              EXCLUDE_FROM_ALL)
  set(singleValueKeywords DIRECTORY TARGET)
  set(multiValueKeywords LINKS PRIVATE_HEADERS PUBLIC_HEADERS SOURCES)
  cmake_parse_arguments(PARSE_ARGV 0 "arg" "${options}"
                        "${singleValueKeywords}" "${multiValueKeywords}")

  if(arg_EXECUTABLE)
    set(type EXECUTABLE)
  endif()
  if(arg_STATIC)
    set(type STATIC)
  endif()
  if(arg_TEST)
    set(type TEST)
  endif()

  if(arg_ENABLE_EXCEPTIONS_IN_COVERAGE)
    set(exceptions ENABLE_EXCEPTIONS_IN_COVERAGE)
  else()
    set(exceptions "")
  endif()

  if(arg_EXCLUDE_FROM_ALL)
    set(exclude_from_all EXCLUDE_FROM_ALL)
  else()
    set(exclude_from_all "")
  endif()

  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(system_name "linux")
  endif()

  list(TRANSFORM arg_SOURCES
       PREPEND ${PROJECT_ROOT_DIR}/${arg_DIRECTORY}/${system_name}/)
  list(TRANSFORM arg_PRIVATE_HEADERS
       PREPEND ${PROJECT_ROOT_DIR}/${arg_DIRECTORY}/internal/)
  list(TRANSFORM arg_PUBLIC_HEADERS
       PREPEND ${PROJECT_ROOT_DIR}/${arg_DIRECTORY}/include/${arg_TARGET}/)

  new_target_(
    ${type}
    ${exceptions}
    ${exclude_from_all}
    TARGET
    ${arg_TARGET}
    DIRECTORY
    ${PROJECT_ROOT_DIR}/${arg_DIRECTORY}
    SOURCES
    ${arg_SOURCES}
    PRIVATE_HEADERS
    ${arg_PRIVATE_HEADERS}
    PUBLIC_HEADERS
    ${arg_PUBLIC_HEADERS}
    LINKS
    ${arg_LINKS})
endfunction()
