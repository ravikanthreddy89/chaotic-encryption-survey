cmake_minimum_required(VERSION 3.16)
project(chaos_image_refsimd C CXX)

include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_SIMD_INTRINSICS "Enable explicit SIMD intrinsics where available" ON)
option(ENABLE_AVX2 "Enable AVX2 intrinsics where available" ON)
option(ENABLE_NATIVE_ARCH "Enable -march=native for host-tuned builds" ON)
option(DISABLE_AUTO_VECTORIZATION "Disable compiler auto-vectorization for scalar reference builds" OFF)
option(ENABLE_OPENMP "Enable OpenMP parallel loops" OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fno-omit-frame-pointer -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")

add_executable(chaos_ref
  src/main.cpp
)

target_include_directories(chaos_ref PRIVATE include)

find_package(OpenCV REQUIRED)
find_package(OpenSSL REQUIRED)

target_include_directories(chaos_ref PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(chaos_ref PRIVATE ${OpenCV_LIBS})

add_library(official_blake3 STATIC
  third_party/blake3/blake3.c
  third_party/blake3/blake3_dispatch.c
  third_party/blake3/blake3_portable.c
)

target_include_directories(official_blake3 PUBLIC third_party/blake3)
target_compile_features(official_blake3 PUBLIC c_std_99)
target_compile_definitions(official_blake3 PRIVATE
  BLAKE3_USE_NEON=0
  BLAKE3_NO_SSE2
  BLAKE3_NO_SSE41
  BLAKE3_NO_AVX2
  BLAKE3_NO_AVX512
)

add_executable(chaos_image_bench
  src/bench_main.cpp
)

target_include_directories(chaos_image_bench PRIVATE
  src
  third_party/blake3
  ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(chaos_image_bench PRIVATE
  ${OpenCV_LIBS}
  OpenSSL::Crypto
  official_blake3
)

string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" CHAOSREF_ARCH_LOWER)
set(CHAOSREF_X86 FALSE)
if(CHAOSREF_ARCH_LOWER MATCHES "x86_64|amd64|i[3-6]86")
  set(CHAOSREF_X86 TRUE)
endif()

if(ENABLE_NATIVE_ARCH AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(chaos_ref PRIVATE -march=native)
  target_compile_options(chaos_image_bench PRIVATE -march=native)
endif()

if(NOT ENABLE_SIMD_INTRINSICS)
  target_compile_definitions(chaos_ref PRIVATE CHAOSREF_DISABLE_SIMD=1)
  target_compile_definitions(chaos_image_bench PRIVATE CHAOSREF_DISABLE_SIMD=1)
endif()

if(DISABLE_AUTO_VECTORIZATION AND CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_compile_options(chaos_ref PRIVATE -fno-tree-vectorize)
  target_compile_options(chaos_image_bench PRIVATE -fno-tree-vectorize)
elseif(DISABLE_AUTO_VECTORIZATION AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(chaos_ref PRIVATE -fno-vectorize -fno-slp-vectorize)
  target_compile_options(chaos_image_bench PRIVATE -fno-vectorize -fno-slp-vectorize)
endif()

if(ENABLE_SIMD_INTRINSICS AND CHAOSREF_X86 AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(chaos_ref PRIVATE -msse2)
  target_compile_options(chaos_image_bench PRIVATE -msse2)
endif()

if(ENABLE_SIMD_INTRINSICS AND ENABLE_AVX2 AND CHAOSREF_X86)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(chaos_ref PRIVATE -mavx2)
    target_compile_options(chaos_image_bench PRIVATE -mavx2)
  endif()
  target_compile_definitions(chaos_ref PRIVATE CHAOSREF_ENABLE_AVX2=1)
  target_compile_definitions(chaos_image_bench PRIVATE CHAOSREF_ENABLE_AVX2=1)
endif()

if(ENABLE_OPENMP)
  find_package(OpenMP REQUIRED)
  if(OpenMP_CXX_FOUND)
    target_link_libraries(chaos_ref PRIVATE OpenMP::OpenMP_CXX)
    target_compile_definitions(chaos_ref PRIVATE CHAOSREF_ENABLE_OPENMP=1)
  endif()
endif()

install(TARGETS chaos_ref DESTINATION bin)
install(TARGETS chaos_image_bench DESTINATION bin)

if(BUILD_TESTING)
  add_executable(chaosref_tests tests/reference_tests.cpp)
  target_include_directories(chaosref_tests PRIVATE include)
  if(ENABLE_NATIVE_ARCH AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(chaosref_tests PRIVATE -march=native)
  endif()
  if(NOT ENABLE_SIMD_INTRINSICS)
    target_compile_definitions(chaosref_tests PRIVATE CHAOSREF_DISABLE_SIMD=1)
  endif()
  if(ENABLE_SIMD_INTRINSICS AND ENABLE_AVX2 AND CHAOSREF_X86)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
      target_compile_options(chaosref_tests PRIVATE -mavx2)
    endif()
    target_compile_definitions(chaosref_tests PRIVATE CHAOSREF_ENABLE_AVX2=1)
  endif()
  add_test(NAME chaosref_reference_tests COMMAND chaosref_tests)

  add_executable(evp_cipher_tests tests/evp_cipher_tests.cpp)
  target_include_directories(evp_cipher_tests PRIVATE src ${OpenCV_INCLUDE_DIRS})
  target_link_libraries(evp_cipher_tests PRIVATE ${OpenCV_LIBS} OpenSSL::Crypto)
  add_test(NAME evp_cipher_tests COMMAND evp_cipher_tests)
endif()
