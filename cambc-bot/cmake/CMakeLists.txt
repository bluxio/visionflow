cmake_minimum_required(VERSION 3.20)
project(bot CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT DEFINED BOT_DIR OR NOT DEFINED BC_SDK)
  message(FATAL_ERROR "BOT_DIR and BC_SDK must be set")
endif()
# Normalize Windows arguments before embedding paths in generated glob checks.
file(TO_CMAKE_PATH "${BOT_DIR}" BOT_DIR)
file(TO_CMAKE_PATH "${BC_SDK}" BC_SDK)
file(GLOB_RECURSE BOT_SOURCES CONFIGURE_DEPENDS "${BOT_DIR}/src/*.cpp")
add_executable(bot ${BOT_SOURCES})
target_include_directories(bot PRIVATE "${BC_SDK}/cpp")
# Make the SDK's exception-enabled libc++ layout visible to clangd versions
# whose built-in driver predates this WASI SDK (without --query-driver setup).
target_include_directories(bot SYSTEM PRIVATE
  "${WASI_SDK_PREFIX}/share/wasi-sysroot/include/wasm32-wasip1/eh/c++/v1"
)
# Wasm exceptions (exnref): the runner's Wasmtime supports only that
# proposal, and wasi-sdk's `eh` sysroot variant (libc++abi/libunwind built
# with try_table/throw_ref) must be linked for C++ exceptions.
target_compile_options(bot PRIVATE -msimd128 -fwasm-exceptions -mllvm -wasm-use-legacy-eh=false)
target_link_options(
  bot PRIVATE -fwasm-exceptions -mllvm -wasm-use-legacy-eh=false
  -L${WASI_SDK_PREFIX}/share/wasi-sysroot/lib/wasm32-wasip1/eh
  -lunwind
)
set_target_properties(bot PROPERTIES SUFFIX ".wasm")
