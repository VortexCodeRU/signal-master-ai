cmake_minimum_required(VERSION 3.20)
project(SignalMasterAI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# For better debug on Linux
set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")

# vcpkg
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

find_package(mongo-cxx-driver CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# httplib — header-only
add_executable(signal_master_ai
    main.cpp
    Assistant.cpp
    MongoDBHelper.cpp
    OllamaClient.cpp
)

target_link_libraries(signal_master_ai PRIVATE
    mongo::mongocxx_shared
    mongo::bsoncxx_shared
    nlohmann_json::nlohmann_json
)

target_include_directories(signal_master_ai PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# succeful flags for Linux
target_compile_options(signal_master_ai PRIVATE -Wall -Wextra -O2)