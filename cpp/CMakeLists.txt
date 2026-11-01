# MalvaUI: the C++ side of the Malva UI kit.
#
#   Malva::UI        header-only: tokens, colour maths, motion, theme, icons (needs only the standard library)
#   malva_add_slint  adds a .slint file to a target with the kit's Slint library importable as "@malva/malva.slint"
#
# Use it from another project:
#     add_subdirectory(path/to/malva-ui-kit/cpp malva)
#     target_link_libraries(myapp PRIVATE Malva::UI Slint::Slint)
#     malva_add_slint(myapp ui/main.slint)        # main.slint:  import { Button } from "@malva/malva.slint";
cmake_minimum_required(VERSION 3.21)
if(NOT PROJECT_NAME)
    project(MalvaUI LANGUAGES CXX)
endif()

get_filename_component(MALVA_KIT_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set(MALVA_SLINT_DIR "${MALVA_KIT_ROOT}/slint" CACHE PATH "The kit's Slint library")

add_library(malva_ui INTERFACE)
add_library(Malva::UI ALIAS malva_ui)
target_include_directories(malva_ui INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")
target_compile_features(malva_ui INTERFACE cxx_std_20)

# malva_add_slint(<target> <file.slint>...): slint_target_sources with the kit's library path. The images and fonts
# of the kit are embedded in the program (so it runs from anywhere) unless the target sets SLINT_EMBED_RESOURCES.
function(malva_add_slint target)
    if(NOT COMMAND slint_target_sources)
        find_package(Slint 1.18 REQUIRED)
    endif()
    if(NOT DEFINED SLINT_EMBED_RESOURCES AND NOT CMAKE_PROJECT_NAME STREQUAL "")
        set_target_properties(${target} PROPERTIES SLINT_EMBED_RESOURCES embed-files)
    endif()
    slint_target_sources(${target} ${ARGN} LIBRARY_PATHS malva=${MALVA_SLINT_DIR})
endfunction()

option(MALVA_BUILD_TESTS "Build the kit's tests" ON)
option(MALVA_BUILD_EXAMPLES "Build the kit's example programs (needs Slint)" ON)
option(MALVA_UI_TEST "Build the interaction test of the Slint components (needs Slint's testing backend)" ON)

if(MALVA_BUILD_TESTS)
    enable_testing()
    foreach(t color_test tokens_test motion_test)
        add_executable(${t} tests/${t}.cpp)
        target_link_libraries(${t} PRIVATE Malva::UI)
        target_compile_options(${t} PRIVATE -Wall -Wextra)
        target_compile_definitions(${t} PRIVATE MALVA_DIST_DIR="${MALVA_KIT_ROOT}/dist")
        add_test(NAME ${t} COMMAND ${t})
    endforeach()
endif()

if((MALVA_BUILD_EXAMPLES OR MALVA_UI_TEST) AND NOT Slint_FOUND)
    find_package(Slint 1.18 QUIET)
endif()

if(Slint_FOUND AND MALVA_UI_TEST AND MALVA_BUILD_TESTS)
    add_executable(ui_test tests/ui_test.cpp)
    malva_add_slint(ui_test tests/testbench.slint)
    target_link_libraries(ui_test PRIVATE Malva::UI Slint::Slint)
    target_include_directories(ui_test PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
    target_compile_options(ui_test PRIVATE -Wall -Wextra)
    add_test(NAME ui_test COMMAND ui_test)
endif()

if(Slint_FOUND AND MALVA_BUILD_EXAMPLES)
    add_executable(malva_gallery examples/gallery_app.cpp)
    malva_add_slint(malva_gallery examples/gallery.slint)
    target_link_libraries(malva_gallery PRIVATE Malva::UI Slint::Slint)
endif()
