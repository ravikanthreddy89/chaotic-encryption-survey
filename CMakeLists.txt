cmake_minimum_required(VERSION 3.16)
project(imgcrypt_bench CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(YEH_USE_FAST_SIN "Use sinf-based YeHuang sine backend (faster, numerically different)" OFF)
set(CHAOS_DIFFUSE_KERNEL "0" CACHE STRING "Diffusion backend: 0=scalar baseline, 1=tiled SIMD-preXOR experiment, 2=exact block-scan (prefix-XOR)")
set_property(CACHE CHAOS_DIFFUSE_KERNEL PROPERTY STRINGS 0 1 2)
set(CHAOS_DIFFUSE_TILE_BYTES "16384" CACHE STRING "Tile size in bytes for CHAOS_DIFFUSE_KERNEL=1 or 2")
option(ENABLE_VEC_REPORT "Enable compiler vectorization report flags" OFF)

# ── Build type ─────────────────────────────────────────────────────────────────
# If not build type passed, default to production build without debug flags
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Reproducible flags: no PGO, no LTO (would make cross-lab comparison unfair)
# -fno-omit-frame-pointer: needed for accurate perf profiling
set(CMAKE_CXX_FLAGS_RELEASE
    "-O3 -march=native -fno-omit-frame-pointer -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG
    "-g -O0 -fsanitize=address,undefined")

message(STATUS "Build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "CXX flags  : ${CMAKE_CXX_FLAGS_RELEASE}")

# ── Dependencies ───────────────────────────────────────────────────────────────
# NOTE: Refer to Notes file on installing the dependencies
find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(OPENSSL REQUIRED openssl)
pkg_check_modules(SODIUM  REQUIRED libsodium)
pkg_check_modules(PAPI    REQUIRED papi)
pkg_check_modules(GSL     REQUIRED gsl)

message(STATUS "OpenCV  : ${OpenCV_VERSION}")
message(STATUS "OpenSSL : ${OPENSSL_VERSION}")
message(STATUS "Sodium  : ${SODIUM_VERSION}")
message(STATUS "PAPI    : ${PAPI_VERSION}")
message(STATUS "GSL     : ${GSL_VERSION}")

# ── Target ─────────────────────────────────────────────────────────────────────
add_executable(imgcrypt_bench main.cpp)

target_include_directories(imgcrypt_bench PRIVATE
    include
    ${OpenCV_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIRS}
    ${SODIUM_INCLUDE_DIRS}
    ${PAPI_INCLUDE_DIRS}
    ${GSL_INCLUDE_DIRS}
)

target_link_libraries(imgcrypt_bench PRIVATE
    ${OpenCV_LIBS}
    ${OPENSSL_LIBRARIES}
    ${SODIUM_LIBRARIES}
    ${PAPI_LIBRARIES}
    ${GSL_LIBRARIES}
)

target_compile_options(imgcrypt_bench PRIVATE
    ${OPENSSL_CFLAGS_OTHER}
    ${SODIUM_CFLAGS_OTHER}
    ${PAPI_CFLAGS_OTHER}
)

if(ENABLE_VEC_REPORT)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(imgcrypt_bench PRIVATE
      -fopt-info-vec-optimized
      -fopt-info-vec-missed
    )
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(imgcrypt_bench PRIVATE
      -Rpass=loop-vectorize
      -Rpass-missed=loop-vectorize
    )
  elseif(MSVC)
    target_compile_options(imgcrypt_bench PRIVATE /Qvec-report:2)
  endif()
endif()

if(YEH_USE_FAST_SIN)
  target_compile_definitions(imgcrypt_bench PRIVATE YEH_USE_FAST_SIN=1)
endif()

target_compile_definitions(imgcrypt_bench PRIVATE
    CHAOS_DIFFUSE_KERNEL=${CHAOS_DIFFUSE_KERNEL}
    CHAOS_DIFFUSE_TILE_BYTES=${CHAOS_DIFFUSE_TILE_BYTES}
)

# ── Install ────────────────────────────────────────────────────────────────────
install(TARGETS imgcrypt_bench DESTINATION bin)

# ── Print compile command (for paper's reproducibility section) ────────────────
add_custom_target(show_flags
    COMMAND ${CMAKE_COMMAND} -E echo "CXX: ${CMAKE_CXX_COMPILER}"
    COMMAND ${CMAKE_COMMAND} -E echo "FLAGS: ${CMAKE_CXX_FLAGS_RELEASE}"
    COMMAND ${CMAKE_COMMAND} -E echo "YEH_USE_FAST_SIN: ${YEH_USE_FAST_SIN}"
    COMMAND ${CMAKE_COMMAND} -E echo "CHAOS_DIFFUSE_KERNEL: ${CHAOS_DIFFUSE_KERNEL}"
    COMMAND ${CMAKE_COMMAND} -E echo "CHAOS_DIFFUSE_TILE_BYTES: ${CHAOS_DIFFUSE_TILE_BYTES}"
    COMMAND ${CMAKE_COMMAND} -E echo "ENABLE_VEC_REPORT: ${ENABLE_VEC_REPORT}"
    VERBATIM
)
