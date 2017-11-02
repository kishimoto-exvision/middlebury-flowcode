set(execName ColorFlow)

cmake_minimum_required(VERSION 3.5)

project(${execName})


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(WIN32)
    # Is '-G "... Win64"' only way to generate files for x64 Platform?
    #add_definitions(-DSYSTEM_PROCESSOR=x64)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES)
    add_compile_options(/Zi /W3 /wd4101 /wd4819 /wd4244)
    set(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "limited configs" FORCE)

    #add_definitions(-DM_PI=3.14159265358979)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG")
else()
    add_definitions(-std=c++11 -g -O3 -Wall)
endif()


find_package(OpenCV COMPONENTS core highgui imgproc imgcodecs REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIBRARY_DIRS})


set(SRC_FOLDER "src")
file(GLOB_RECURSE SRC_FILES "${SRC_FOLDER}/*.cpp" "${SRC_FOLDER}/*.h" "${SRC_FOLDER}/*.txt")
add_executable(${execName} ${SRC_FILES})
target_link_libraries(${execName} ${OpenCV_LIBRARIES})
