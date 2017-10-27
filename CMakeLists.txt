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


set(ORIGINAL_DIR "original")
set(IMAGELIB_DIR "original/imageLib")


find_package(PNG REQUIRED)
include_directories(${IMAGELIB_DIR})
include_directories(${PNG_INCLUDE_DIR})
file(GLOB IMAGE_LIB_SRC "${IMAGELIB_DIR}/*.cpp")
add_library("imageLib" STATIC ${IMAGE_LIB_SRC})
target_link_libraries("imageLib" ${PNG_LIBRARY})


include_directories(${ORIGINAL_DIR})
set(COLORTEST_SRC ${ORIGINAL_DIR}/colortest.cpp ${ORIGINAL_DIR}/colorcode.cpp)
add_executable("colortest" ${COLORTEST_SRC})
target_link_libraries("colortest" imageLib)


set(COLOR_FLOW_SRC ${ORIGINAL_DIR}/color_flow.cpp ${ORIGINAL_DIR}/flowIO.cpp ${ORIGINAL_DIR}/colorcode.cpp)
add_executable("color_flow" ${COLOR_FLOW_SRC})
target_link_libraries("color_flow" imageLib)


file(GLOB_RECURSE FLOWCODE_HEADER "${ORIGINAL_DIR}/*.h")
add_executable("colorflow"
    ${FLOWCODE_HEADER}
    ${ORIGINAL_DIR}/flowIO.cpp
    ${ORIGINAL_DIR}/colorcode.cpp
    "colorflow/colorflow.cpp")
target_link_libraries("colorflow" imageLib)
