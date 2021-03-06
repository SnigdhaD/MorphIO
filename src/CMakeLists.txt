set(MORPHIO_SOURCES
    enums.cpp
    errorMessages.cpp
    mito_section.cpp
    mitochondria.cpp
    morphology.cpp
    morphology.cpp
    properties.cpp
    section.cpp
    soma.cpp
    vector_utils.cpp
    version.cpp
    mut/mito_section.cpp
    mut/section.cpp
    mut/soma.cpp
    mut/morphology.cpp
    mut/mitochondria.cpp
    mut/writers.cpp
    mut/modifiers.cpp
    readers/morphologyHDF5.cpp
    readers/morphologySWC.cpp
    readers/morphologyASC.cpp
    readers/vasculatureHDF5.cpp
    vasc/section.cpp
    vasc/vasculature.cpp
    vasc/properties.cpp
    )

if(NOT MORPHIO_VERSION_STRING)
    execute_process(COMMAND git describe --tags
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    RESULT_VARIABLE GIT_VERSION_FAILED
                    OUTPUT_VARIABLE MORPHIO_VERSION_STRING
                    ERROR_VARIABLE GIT_VERSION_ERROR
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp.in
  ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp
  )

# by default, -fPIC is only used of the dynamic library build
# This forces the flag also for the static lib
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Building object files only once. They will be used for the shared and static library
add_library(morphio_obj OBJECT ${MORPHIO_SOURCES})

target_include_directories(morphio_obj
  PUBLIC
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  )

target_include_directories(morphio_obj
  SYSTEM
  PUBLIC
   $<TARGET_PROPERTY:gsl-lite,INTERFACE_INCLUDE_DIRECTORIES>
   $<TARGET_PROPERTY:HighFive,INTERFACE_INCLUDE_DIRECTORIES>
   $<TARGET_PROPERTY:lexertl,INTERFACE_INCLUDE_DIRECTORIES>
  )

set_target_properties(morphio_obj
  PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  )

target_compile_options(morphio_obj
  PRIVATE
  -Wall
  -Wextra
  -Wnon-virtual-dtor
  -pedantic
  )

add_library(morphio_static STATIC $<TARGET_OBJECTS:morphio_obj>)
add_library(morphio_shared SHARED $<TARGET_OBJECTS:morphio_obj>)
set_target_properties(morphio_shared PROPERTIES OUTPUT_NAME "morphio"
                                                EXPORT_NAME "morphio")

target_include_directories(morphio_static
  SYSTEM
  PUBLIC
   $<TARGET_PROPERTY:gsl-lite,INTERFACE_INCLUDE_DIRECTORIES>
  PRIVATE
   $<TARGET_PROPERTY:HighFive,INTERFACE_INCLUDE_DIRECTORIES>
   $<TARGET_PROPERTY:lexertl,INTERFACE_INCLUDE_DIRECTORIES>
  )

target_link_libraries(morphio_static PUBLIC gsl-lite PRIVATE HighFive lexertl)
target_link_libraries(morphio_shared PUBLIC gsl-lite PRIVATE HighFive lexertl)

install(
  # DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  TARGETS morphio_shared
  EXPORT MorphIOTargets
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include
  )
