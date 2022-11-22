# WASM Serverless CPP SDK
This is the C++ SDK for building a mimik WASM Serverless microservice

# Instruction for using this with CMAKE
Add the following lines to the CMakeLists.txt in order to fetch the SDK, and then use the sdk include folder.
```
include(FetchContent)

Set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
  serverless-sdk
  GIT_REPOSITORY https://github.com/edgeMicroservice/wasm-serverless-cpp-sdk.git
  GIT_TAG ${SDK_TAG}
  GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(serverless-sdk)

include_directories("${serverless-sdk_SOURCE_DIR}/include")
```
<br />  

Add the following lines to the CMakeLists.txt to complete setting up the dependency on the SDK 
```
target_link_libraries(${PROJECT_NAME}
  wasm-serverless
)
```
