# The smallest program that uses the kit from outside: a standalone CMake project that adds the kit's cpp/ folder and
# one .slint file importing "@malva/malva.slint".
#
#   cmake -S cpp/examples/consumer -B build-consumer -DCMAKE_PREFIX_PATH=/path/to/Slint-cpp-1.18.1-Linux-x86_64
#   cmake --build build-consumer && ./build-consumer/consumer --run     # needs a display
cmake_minimum_required(VERSION 3.21)
project(malva-consumer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Slint 1.18 REQUIRED)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../.. malva)      # Malva::UI and malva_add_slint()

add_executable(consumer main.cpp)
malva_add_slint(consumer app.slint)
target_link_libraries(consumer PRIVATE Malva::UI Slint::Slint)
