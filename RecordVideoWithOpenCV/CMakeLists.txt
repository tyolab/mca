# CMakeList.txt : CMake project for CMakeProject1, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)


find_package(OpenCV REQUIRED)

find_package(Pylon QUIET)
if (NOT ${Pylon_FOUND})
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindPylon.cmake")
endif()

include_directories(
    ${PYLON_ROOT_DIR}/include
    $ENV{OPENCV_DIR}/include
    )

link_directories(
    ${PYLON_ROOT_DIR}/lib/x86
    $ENV{OPENCV_DIR}/build/x64/vc15/lib
    )

# Add source to this project's executable.
add_executable (record "main.cpp")
target_link_libraries(record ${PYLON_LIBRARIES})

# TODO: Add tests and install targets if needed.
