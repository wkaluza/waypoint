# Create new target and configure it
function(new_target)
  set(options EXECUTABLE STATIC TEST)
  set(singleValueKeywords TARGET DIRECTORY)
  set(multiValueKeywords SOURCES INTERNAL_HEADERS INTERFACE_HEADERS LINKS)
  cmake_parse_arguments(PARSE_ARGV 0 "arg" "${options}"
                        "${singleValueKeywords}" "${multiValueKeywords}")

  if((arg_EXECUTABLE AND arg_STATIC)
     OR (arg_EXECUTABLE AND arg_TEST)
     OR (arg_TEST AND arg_STATIC))
    message(
      FATAL_ERROR
        "new_target: EXECUTABLE, TEST and STATIC are mutually exclusive")
  endif()

  if(arg_EXECUTABLE)
    add_executable(${arg_TARGET})
  elseif(arg_TEST)
    add_executable(${arg_TARGET})
    add_test(NAME test_${arg_TARGET} COMMAND ${arg_TARGET})
  elseif(arg_STATIC)
    add_library(${arg_TARGET} STATIC)
  endif()

  target_compile_features(${arg_TARGET} PRIVATE cxx_std_20)

  if(DEFINED arg_SOURCES)
    target_sources(${arg_TARGET} PRIVATE ${arg_SOURCES})
  endif()

  if(DEFINED arg_LINKS)
    target_link_libraries(${arg_TARGET} PRIVATE ${arg_LINKS})
  endif()

  if(DEFINED arg_DIRECTORY)
    if(DEFINED arg_INTERNAL_HEADERS)
      target_sources(
        ${arg_TARGET}
        PRIVATE FILE_SET
                internal_headers_${arg_TARGET}
                TYPE
                HEADERS
                BASE_DIRS
                ${arg_DIRECTORY}/internal
                FILES
                ${arg_INTERNAL_HEADERS})
    endif()

    if(DEFINED arg_INTERFACE_HEADERS)
      target_sources(
        ${arg_TARGET}
        PRIVATE FILE_SET
                own_headers_${arg_TARGET}
                TYPE
                HEADERS
                BASE_DIRS
                ${arg_DIRECTORY}/include/${arg_TARGET}
                FILES
                ${arg_INTERFACE_HEADERS})

      target_sources(
        ${arg_TARGET}
        INTERFACE FILE_SET
                  interface_headers_${arg_TARGET}
                  TYPE
                  HEADERS
                  BASE_DIRS
                  ${arg_DIRECTORY}/include
                  FILES
                  ${arg_INTERFACE_HEADERS})
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
