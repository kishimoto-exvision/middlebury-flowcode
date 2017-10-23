set(execName flow_code)

cmake_minimum_required(VERSION 3.1)

project(${execName})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
    add_compile_options(/Zi /W3 /D_CRT_SECURE_NO_WARNINGS /DM_PI=3.14159265358979 /wd4101 /wd4819 /wd4244)
else()
    add_definitions(-std=c++0x -g -O3) # -m64)
endif()


find_package(PNG REQUIRED)
include_directories("imageLib")
include_directories(${PNG_INCLUDE_DIR})
file(GLOB IMAGE_LIB_SRC "imageLib/*.cpp")
add_library("imageLib" STATIC ${IMAGE_LIB_SRC})
target_link_libraries("imageLib" ${PNG_LIBRARY})


include_directories(".")
set(COLORTEST_SRC colortest.cpp colorcode.cpp)
add_executable("colortest" ${COLORTEST_SRC})
target_link_libraries("colortest" imageLib)


set(COLOR_FLOW_SRC color_flow.cpp flowIO.cpp colorcode.cpp)
add_executable("color_flow" ${COLOR_FLOW_SRC})
target_link_libraries("color_flow" imageLib)
