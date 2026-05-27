cmake_minimum_required(VERSION 3.16)
project(chaos_image_refsimd CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_AVX2 "Enable AVX2 intrinsics where available" ON)
option(ENABLE_OPENMP "Enable OpenMP parallel loops" OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -fno-omit-frame-pointer -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")

add_executable(chaos_ref
  src/main.cpp
)

target_include_directories(chaos_ref PRIVATE include)

string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" CHAOSREF_ARCH_LOWER)
set(CHAOSREF_X86 FALSE)
if(CHAOSREF_ARCH_LOWER MATCHES "x86_64|amd64|i[3-6]86")
  set(CHAOSREF_X86 TRUE)
endif()

if(ENABLE_AVX2 AND CHAOSREF_X86)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(chaos_ref PRIVATE -mavx2)
  endif()
  target_compile_definitions(chaos_ref PRIVATE CHAOSREF_ENABLE_AVX2=1)
endif()

if(ENABLE_OPENMP)
  find_package(OpenMP REQUIRED)
  if(OpenMP_CXX_FOUND)
    target_link_libraries(chaos_ref PRIVATE OpenMP::OpenMP_CXX)
    target_compile_definitions(chaos_ref PRIVATE CHAOSREF_ENABLE_OPENMP=1)
  endif()
endif()

install(TARGETS chaos_ref DESTINATION bin)
