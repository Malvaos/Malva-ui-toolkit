# The Malva UI kit. Everything of the C++ side is under cpp/; the Rust workspace is rust/ (cargo test).
cmake_minimum_required(VERSION 3.21)
project(malva-ui-kit LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
enable_testing()

# The Files app, the dock and the menu bar, built on the kit (needs Slint 1.18 and a C++20 compiler).
option(MALVA_BUILD_APPS "Build the Files, Dock and Menu bar apps and their tests (needs Slint)" OFF)
# Slint is found once, here, for the kit's examples and tests and for the apps (its imported targets can only be made once).
if(MALVA_BUILD_APPS)
    find_package(Slint 1.18 REQUIRED)
else()
    find_package(Slint 1.18 QUIET)
endif()

add_subdirectory(cpp)
if(MALVA_BUILD_APPS)
    add_subdirectory(apps)
endif()
