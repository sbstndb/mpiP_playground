cmake_minimum_required(VERSION 3.15)
project(MPIBottleneck LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(MPI REQUIRED)

set(MPIP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/mpiP/install")
find_library(MPIP_LIBRARY NAMES mpiP PATHS "${MPIP_ROOT}/lib" NO_DEFAULT_PATH)
if (NOT MPIP_LIBRARY)
  message(FATAL_ERROR "mpiP library not found. Did you run configure && make install in third_party/mpiP?")
endif()

add_executable(bottleneck
  src/main.cpp
  src/simple/pipeline.cpp
  src/simple/workload.cpp
)
add_executable(nested_bottleneck
  src/nested/main.cpp
  src/nested/pipeline.cpp
  src/nested/communication.cpp
  src/nested/workers.cpp
)

set(MPIP_TARGETS bottleneck nested_bottleneck)

foreach(target IN LISTS MPIP_TARGETS)
  target_link_directories(${target} PRIVATE "${MPIP_ROOT}/lib")
  target_link_libraries(${target} PRIVATE MPI::MPI_CXX ${MPIP_LIBRARY} unwind)
  target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${target} PRIVATE -fno-omit-frame-pointer -fno-pie)
    target_link_options(${target} PRIVATE -no-pie)
  endif()
  set_target_properties(${target} PROPERTIES
    BUILD_RPATH "${MPIP_ROOT}/lib"
    INSTALL_RPATH "${MPIP_ROOT}/lib"
  )
  target_compile_options(${target} PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,Intel>: -Wall -Wextra -Wpedantic>
  )
endforeach()
