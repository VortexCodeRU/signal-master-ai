set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING>
endif()

list(APPEND CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/arm64-linux/share")

find_package(bsoncxx CONFIG REQUIRED)
find_package(mongocxx CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

find_path(HTTPLIB_INCLUDE_DIR "httplib.h"
    PATHS "$ENV{VCPKG_ROOT}/installed/arm64-linux/include"
    NO_DEFAULT_PATH
    REQUIRED
)

add_executable(signal_master_ai

    main.cpp
    Assistant.cpp
    MongoDBHelper.cpp
    OllamaClient.cpp
)

target_include_directories(signal_master_ai PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${HTTPLIB_INCLUDE_DIR}
)

target_link_libraries(signal_master_ai PRIVATE
    $<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,mongo::bsoncxx_static,mongo::bsoncxx_shared>
    $<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,mongo::mongocxx_static,mongo::mongocxx_share>
    nlohmann_json::nlohmann_json
    pthread
)

target_compile_options(signal_master_ai PRIVATE -O2)
