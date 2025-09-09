include_guard(GLOBAL)

set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

function(new_target_)
  set(options STATIC TEST)
  set(singleValueKeywords DIRECTORY TARGET)
  set(multiValueKeywords LINKS PRIVATE_HEADERS PUBLIC_HEADERS SOURCES)
  cmake_parse_arguments(PARSE_ARGV 0 "arg" "${options}"
                        "${singleValueKeywords}" "${multiValueKeywords}")

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
  endif()
  if(arg_STATIC)
    add_library(${arg_TARGET} STATIC)
  endif()

  target_compile_features(
    ${arg_TARGET}
    PRIVATE cxx_std_23
    PUBLIC cxx_std_17)

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
      add_library(library_own_headers_${arg_TARGET} INTERFACE)
      target_sources(
        library_own_headers_${arg_TARGET}
        INTERFACE FILE_SET
                  own_headers_${arg_TARGET}
                  TYPE
                  HEADERS
                  BASE_DIRS
                  ${arg_DIRECTORY}/include/${arg_TARGET}
                  FILES
                  ${arg_PUBLIC_HEADERS})

      add_library(library_interface_headers_${arg_TARGET} INTERFACE)
      target_sources(
        library_interface_headers_${arg_TARGET}
        INTERFACE FILE_SET
                  interface_headers_${arg_TARGET}
                  TYPE
                  HEADERS
                  BASE_DIRS
                  ${arg_DIRECTORY}/include
                  FILES
                  ${arg_PUBLIC_HEADERS})

      target_link_libraries(
        ${arg_TARGET}
        PUBLIC $<BUILD_LOCAL_INTERFACE:library_interface_headers_${arg_TARGET}>
        PRIVATE $<BUILD_LOCAL_INTERFACE:library_own_headers_${arg_TARGET}>)
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

function(new_target)
  set(options STATIC TEST)
  set(singleValueKeywords DIRECTORY TARGET)
  set(multiValueKeywords LINKS PRIVATE_HEADERS PUBLIC_HEADERS SOURCES)
  cmake_parse_arguments(PARSE_ARGV 0 "arg" "${options}"
                        "${singleValueKeywords}" "${multiValueKeywords}")

  if(arg_STATIC)
    set(type STATIC)
  endif()
  if(arg_TEST)
    set(type TEST)
  endif()

  list(TRANSFORM arg_SOURCES PREPEND ${PROJECT_ROOT_DIR}/${arg_DIRECTORY}/)
  list(TRANSFORM arg_PRIVATE_HEADERS
       PREPEND ${PROJECT_ROOT_DIR}/${arg_DIRECTORY}/internal/)
  list(TRANSFORM arg_PUBLIC_HEADERS
       PREPEND ${PROJECT_ROOT_DIR}/${arg_DIRECTORY}/include/${arg_TARGET}/)

  new_target_(
    ${type}
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
