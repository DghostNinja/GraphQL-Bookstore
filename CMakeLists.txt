cmake_minimum_required(VERSION 3.15)
project(VulnerableGraphQLBookstore VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

pkg_check_modules(libpq REQUIRED libpq)
include_directories(${libpq_INCLUDE_DIRS})

# OpenSSL
find_package(OpenSSL REQUIRED)

find_library(JWT_LIB jwt)
find_path(JWT_INCLUDE jwt.h)

pkg_check_modules(CURL REQUIRED libcurl)

# nlohmann_json
pkg_check_modules(nlohmann_json3 IMPORTED_TARGET nlohmann_json3)

set(SOURCES
    src/main.cpp
    src/database/connection.cpp
)

add_executable(bookstore-api ${SOURCES})

target_link_libraries(bookstore-api
    ${libpq_LIBRARIES}
    ${JWT_LIB}
    ${CURL_LIBRARIES}
    ${nlohmann_json3_LIBRARIES}
    pthread
    dl
)

install(TARGETS bookstore-api DESTINATION bin)