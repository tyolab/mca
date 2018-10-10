# CMakeList.txt : CMake project for mca, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake-modules)

find_package(OpenCV REQUIRED)

# Add source to this project's executable.
add_executable (mca "mca.cpp" "mca.h")


# TODO: Add tests and install targets if needed.
