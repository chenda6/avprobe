cmake_minimum_required(VERSION 3.17)

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

#add_executable(${PROJECT_NAME}
#    ffpacket.cpp
#    prober.cpp
#    main.cpp
#)

add_library(${PROJECT_NAME} SHARED
    ffpacket.cpp
    prober.cpp
    avprobe_wrap.cxx
)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")

target_compile_options(${PROJECT_NAME} PRIVATE
  -std=c++17 -Wall -fpermissive -Wno-deprecated-declarations)

target_link_libraries(${PROJECT_NAME} PUBLIC
    PkgConfig::LIBAV
)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)