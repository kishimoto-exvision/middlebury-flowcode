set(execName Flowcode)

cmake_minimum_required(VERSION 3.5)

project(${execName})

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "./install" CACHE PATH "install folder prefix" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(WIN32)
    # Is '-G "... Win64"' only way to generate files for x64 Platform?
    #add_definitions(-DSYSTEM_PROCESSOR=x64)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -DM_PI=3.14159265358979)
    add_compile_options(/Zi /W3 /wd4101 /wd4819 /wd4244)
    set(CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo" CACHE STRING "limited configs" FORCE)
else()
    add_definitions(-std=c++11 -g -O3 -Wall)
endif()


set(ORIGINAL_DIR "original")
set(IMAGELIB_DIR "original/imageLib")

find_package(PNG REQUIRED)
set(FlowcodeImageLib_Name "imageLib")
include_directories(${IMAGELIB_DIR})
include_directories(${PNG_INCLUDE_DIR})
file(GLOB IMAGE_LIB_SRC "${IMAGELIB_DIR}/*.cpp")
add_library(${FlowcodeImageLib_Name} STATIC ${IMAGE_LIB_SRC})
target_link_libraries(${FlowcodeImageLib_Name} ${PNG_LIBRARY})


include_directories(${ORIGINAL_DIR})
set(COLORTEST_SRC ${ORIGINAL_DIR}/colortest.cpp ${ORIGINAL_DIR}/colorcode.cpp)
add_executable("colortest" ${COLORTEST_SRC})
target_link_libraries("colortest" ${FlowcodeImageLib_Name})


set(COLOR_FLOW_SRC ${ORIGINAL_DIR}/color_flow.cpp ${ORIGINAL_DIR}/flowIO.cpp ${ORIGINAL_DIR}/colorcode.cpp)
add_executable("color_flow" ${COLOR_FLOW_SRC})
target_link_libraries("color_flow" ${FlowcodeImageLib_Name})


set(Flowcode_Name "Flowcode")
include_directories(${IMAGELIB_DIR})
include_directories(${PNG_INCLUDE_DIR})
file(GLOB_RECURSE FLOWCODE_HEADERS "${IMAGELIB_DIR}/*.h" ${ORIGINAL_DIR}/flowIO.h ${ORIGINAL_DIR}/colorcode.h)
file(GLOB_RECURSE FLOWCODE_SRC "${IMAGELIB_DIR}/*.cpp" ${ORIGINAL_DIR}/flowIO.cpp ${ORIGINAL_DIR}/colorcode.cpp)
add_library(${Flowcode_Name} STATIC ${FLOWCODE_HEADERS} ${FLOWCODE_SRC})
target_link_libraries(${Flowcode_Name} ${PNG_LIBRARY})
set_target_properties(${Flowcode_Name} PROPERTIES PUBLIC_HEADER "${FLOWCODE_HEADERS}")
install(
    TARGETS ${Flowcode_Name}
    EXPORT ${Flowcode_Name}Export
    ARCHIVE        DESTINATION staticlib
    LIBRARY        DESTINATION lib
    INCLUDES       DESTINATION include/${Flowcode_Name}
    PUBLIC_HEADER  DESTINATION include/${Flowcode_Name}
    RUNTIME        DESTINATION bin
)
install(
    EXPORT "${Flowcode_Name}Export"
    FILE "${Flowcode_Name}Config.cmake"
    DESTINATION "share/${Flowcode_Name}/"
    EXPORT_LINK_INTERFACE_LIBRARIES
)


set(ColorFlow_NAME "colorflow")
add_executable(${ColorFlow_NAME} ${FLOWCODE_HEADERS} "colorflow/colorflow.cpp")
target_link_libraries(${ColorFlow_NAME} ${Flowcode_Name})
install(TARGETS ${ColorFlow_NAME} DESTINATION bin)
