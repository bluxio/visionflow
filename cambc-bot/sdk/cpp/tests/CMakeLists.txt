cmake_minimum_required(VERSION 3.20)
project(check_cpp_sdk CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
enable_testing()

# Compile the SDK/engine contract separately; never link engine code into the bot.
get_filename_component(BC_SDK "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
get_filename_component(BC_GAME_ROOT "${BC_SDK}/../.." ABSOLUTE)
add_library(check_cpp_sdk OBJECT "${BC_SDK}/cpp/tests/engine_contract.cpp")
target_include_directories(check_cpp_sdk PRIVATE
  "${BC_SDK}/cpp"
  "${BC_GAME_ROOT}/Engine/include/private_cpp"
  "${BC_GAME_ROOT}/Engine/include/public"
  "${BC_GAME_ROOT}/Engine/include/public_c"
  "${BC_GAME_ROOT}/Deep/Deep/include/public_cpp")
target_compile_definitions(check_cpp_sdk PRIVATE
  _Static_assert=static_assert ENGINE_VALIDATE_C_ABI DEEP_ARCH_WASM DEEP_CPP_EXCEPTIONS_ENABLED=1)
target_compile_options(check_cpp_sdk PRIVATE -msimd128)

# Typed SDK tests against fake host imports, run under wasmtime with the
# same exnref exception flags as bots (cmake/CMakeLists.txt).
add_executable(typed_api "${BC_SDK}/cpp/tests/typed_api.cpp")
target_include_directories(typed_api PRIVATE "${BC_SDK}/cpp")
target_compile_options(typed_api PRIVATE -msimd128 -fwasm-exceptions -mllvm -wasm-use-legacy-eh=false)
target_link_options(
  typed_api PRIVATE -fwasm-exceptions -mllvm -wasm-use-legacy-eh=false
  -L${WASI_SDK_PREFIX}/share/wasi-sysroot/lib/wasm32-wasip1/eh
  -lunwind
)
set_target_properties(typed_api PROPERTIES SUFFIX ".wasm")
find_program(WASMTIME wasmtime REQUIRED)
add_test(NAME typed_api COMMAND "${WASMTIME}" run -W exceptions=y "$<TARGET_FILE:typed_api>")
