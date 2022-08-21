cmake_minimum_required(VERSION 3.16)

project(prober)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    #libavdevice
    #libavfilter
    libavformat
    libavcodec
    #libswresample
    #libswscale
    #libavutil
)

add_executable(${PROJECT_NAME}
    ffpacket.cpp
    prober.cpp
    main.cpp
)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")

target_compile_options(${PROJECT_NAME} PRIVATE
  -std=c++17 -Wall -fpermissive -Wno-deprecated-declarations)

target_link_libraries(${PROJECT_NAME}
    PkgConfig::LIBAV
    #${SDL_LIBRARIES}
    #${SDL2_LIBRARIES}
)

#include_directories(/usr/local/include)
#link_directories(/usr/local/lib)