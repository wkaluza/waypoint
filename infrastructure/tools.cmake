# Create new target and configure it
function(new_target)
  set(options EXECUTABLE STATIC TEST)
  set(singleValueKeywords DIRECTORY TARGET)
  set(multiValueKeywords LINKS PRIVATE_HEADERS PUBLIC_HEADERS SOURCES)
  cmake_parse_arguments(PARSE_ARGV 0 "arg" "${options}"
                        "${singleValueKeywords}" "${multiValueKeywords}")

  if(arg_EXECUTABLE)
    add_executable(${arg_TARGET})
  endif()
  if(arg_TEST)
    add_executable(${arg_TARGET})

    add_test(NAME test_${arg_TARGET} COMMAND $<TARGET_FILE:${arg_TARGET}>)
    set_tests_properties(test_${arg_TARGET} PROPERTIES LABELS test)

    add_test(
      NAME valgrind_${arg_TARGET}
      COMMAND valgrind --leak-check=yes --error-exitcode=1
              $<TARGET_FILE:${arg_TARGET}>
      CONFIGURATIONS Debug)
    set_tests_properties(valgrind_${arg_TARGET} PROPERTIES LABELS valgrind)
  endif()
  if(arg_STATIC)
    add_library(${arg_TARGET} STATIC)
  endif()

  target_compile_features(
    ${arg_TARGET}
    PRIVATE cxx_std_23
    PUBLIC cxx_std_17)

  if(DEFINED PRESET_ENABLE_COVERAGE)
    target_compile_options(${arg_TARGET} PRIVATE ${PRESET_ENABLE_COVERAGE})
    target_link_options(${arg_TARGET} PRIVATE ${PRESET_ENABLE_COVERAGE})

    target_compile_options(${arg_TARGET} PRIVATE -fno-exceptions)
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
    ${PROJECT_ROOT_DIR}/test/${name}/main.cpp
    LINKS
    waypoint
    test_helpers)
endfunction()

function(new_impl_test name)
  new_basic_test(${name})
  target_include_directories(
    ${name} PRIVATE ${PROJECT_ROOT_DIR}/src/waypoint/internal
                    ${PROJECT_ROOT_DIR}/src/waypoint/include/waypoint)
endfunction()
