
cmake_minimum_required(VERSION 3.8)

project(wasm-serverless)

set (CMAKE_CXX_STANDARD 11)

include(FetchContent)
 
FetchContent_Declare(
  cppjson
  URL https://github.com/nlohmann/json/archive/refs/tags/v3.10.4.tar.gz
  # GIT_REPOSITORY https://github.com/nlohmann/json.git
  # GIT_TAG v3.10.4
)

FetchContent_MakeAvailable(cppjson)

add_library(wasm-serverless
  ${CMAKE_CURRENT_SOURCE_DIR}/src/api.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/init.cpp
)

set_target_properties(${PROJECT_NAME}
  PROPERTIES COMPILE_FLAGS "-fno-exceptions"
)

target_include_directories(
  wasm-serverless
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${cppjson_SOURCE_DIR}/single_include
)
