cmake_minimum_required(VERSION 3.20)
project(lyrics_translator CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git GIT_TAG 1.11.0)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_Declare(keychain GIT_REPOSITORY https://github.com/hrantzsch/keychain.git GIT_TAG master)
FetchContent_MakeAvailable(cpr json keychain)

add_executable(app main.cpp caching.cpp Songinfo.cpp Secrets/secrets.cpp Secrets/SpotifyToken.cpp)
find_package(OpenSSL REQUIRED)
target_link_libraries(app PRIVATE cpr::cpr nlohmann_json::nlohmann_json keychain OpenSSL::Crypto)
