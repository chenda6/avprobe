cmake_minimum_required(VERSION 3.17)

project(prober)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavformat
    libavcodec
)

add_executable(${PROJECT_NAME}
    main.cpp
    ../ffpacket.cpp
    ../prober.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")

target_compile_options(${PROJECT_NAME} PRIVATE
  -std=c++17 -Wall -fpermissive -Wno-deprecated-declarations)

target_link_libraries(${PROJECT_NAME} PUBLIC
    PkgConfig::LIBAV
)
