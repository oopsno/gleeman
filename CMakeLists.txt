cmake_minimum_required (VERSION 3.5)
project (gleeman CXX)

set(CMAKE_CXX_STANDARD 14)

option(USE_NVML "enable NVML support")
option(USE_NAT  "enable NAT  support")

set(CUDA_USE_STATIC_CUDA_RUNTIME NO)
find_package(CUDA 8.0 QUIET)
if (CUDA_FOUND)
  set(USE_CUDA YES)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_CUDA")
  set(CUDA_RUNTIME ${CUDA_LIBRARIES})
  if (WIN32)
    string(REPLACE include "lib/x64" CUDA_LIB_DIR ${CUDA_INCLUDE_DIRS})
    set(CUDA_DRIVER   "${CUDA_LIB_DIR}/cuda.lib")
    set(CUDA_NVML     "${CUDA_LIB_DIR}/nvml.lib")
    set(CUDA_NVML_DLL "${NVSMI_ROOT}/nvml.dll")
  elseif (UNIX AND NOT APPLE)
    string(REPLACE include lib64 CUDA_LIB_DIR ${CUDA_INCLUDE_DIRS})
    set(CUDA_DRIVER   "${CUDA_LIB_DIR}/stubs/libcuda.so")
    set(CUDA_NVML     "${CUDA_LIB_DIR}/stubs/libnvidia-ml.so")
    set(CUDA_NVML_DLL ${CUDA_NVML})
  elseif(APPLE)
    set(USE_CUDA NO)
  endif()
  list(APPEND GLEEMAN_CUDA_LIBRAIRES ${CUDA_DRIVER})
  list(APPEND GLEEMAN_CUDA_LIBRAIRES ${CUDA_RUNTIME})
  if (EXISTS ${CUDA_NVML_DLL})
    set(USE_NVML YES)
    list(APPEND GLEEMAN_CUDA_LIBRAIRES ${CUDA_NVML})
  else()
    set(USE_NVML NO)
  endif()
  include_directories(${CUDA_INCLUDE_DIRS})
else()
  set(USE_CUDA NO)
  set(USE_NVML NO)
endif()

message(STATUS ">>> USE_CUDA ${USE_CUDA}")
message(STATUS ">>> USE_NVML ${USE_NVML}")

set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
find_package(Boost QUIET REQUIRED COMPONENTS program_options)

if (UNIX)
  set(BUILD_SHARED_LIBS YES)
else()
  set(BUILD_SHARED_LIBS NO)
endif()
add_subdirectory(3rd/yaml-cpp)
add_subdirectory(3rd/cryptopp)
add_subdirectory(3rd/Catch)
if (BUILD_SHARED_LIBS)
  set(GLEEMAN_LINKING  SHARED)
  set(GLEEMAN_CRYPTOPP cryptopp-shared)
else()
  set(GLEEMAN_LINKING STATIC)
  set(GLEEMAN_CRYPTOPP cryptopp-static)
endif()

set(PROJECT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
include_directories(${PROJECT_INCLUDE_DIR}
                    ${Boost_INCLUDE_DIRS}
                    3rd/yaml-cpp/include
                    3rd/cryptopp
                    3rd/Catch/single_include)

if (USE_NVML)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_NVML")
endif( )

if (USE_NAT)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_NAT")
endif()

set(GLEEMAN_SOURCES src/gleeman/error.cpp
                    src/gleeman/exception.cpp
                    src/gleeman/context.cpp
                    src/gleeman/crypto.cpp
                    src/gleeman/job.cpp
                    src/gleeman/yaml.cpp
                    src/gleeman/executable.cpp
                    src/gleeman/nat.cpp
                    src/gleeman/device.cpp
                    src/gleeman/placeholder.cpp)
add_library(glee ${GLEEMAN_LINKING} ${GLEEMAN_SOURCES})
target_link_libraries(glee ${GLEEMAN_CRYPTOPP} yaml-cpp ${GLEEMAN_CUDA_LIBRAIRES})

add_executable(gleeman src/gleeman.cpp)
target_link_libraries(gleeman glee ${Boost_LIBRARIES})

enable_testing()
set(CATCH_MAIN test/catch_main.cpp)

add_executable(test_traits test/traits.cpp ${CATCH_MAIN})
add_test(NAME test_traits COMMAND test_traits)

add_executable(test_nat test/nat.cpp ${CATCH_MAIN})
add_test(NAME test_nat COMMAND test_nat)

add_executable(test_error test/error.cpp ${CATCH_MAIN})
target_link_libraries(test_error glee)
add_test(NAME test_error COMMAND test_error)

add_executable(test_context test/context.cpp ${CATCH_MAIN})
target_link_libraries(test_context glee)
add_test(NAME test_context COMMAND test_context)

file(GLOB TEST_SOURCES test/*.cpp)
add_executable(test_all ${TEST_SOURCES})
target_link_libraries(test_all glee)

if (WIN32 AND USE_NVML)
  message(">>> Copying nvml.dll [${CUDA_NVML_DLL}]")
  file(COPY ${CUDA_NVML_DLL} DESTINATION ${CMAKE_BINARY_DIR})
endif()
