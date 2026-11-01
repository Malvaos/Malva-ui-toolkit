cmake_minimum_required(VERSION 3.21)
project(menubar-ui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Slint's C++ SDK: https://github.com/slint-ui/slint/releases (Slint-cpp-*-Linux-x86_64.tar.gz),
# found through CMAKE_PREFIX_PATH or Slint_DIR.
if(NOT Slint_FOUND) # already found when the kit builds the apps together
    find_package(Slint 1.18 REQUIRED)
endif()

# ---- The UI: ui/menubar.slint compiled once into a library, with its icons embedded. A backend links it
# and includes "menubar.h" for the generated MenubarWindow and the structs it passes (MenuEntry, Notice, ...).
add_library(barui_ui STATIC backend/ui_anchor.cpp)
# The Malva kit's Slint library, importable as "@malva/malva.slint" (a copy of the kit sits two levels up).
set(MALVA_SLINT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../slint" CACHE PATH "The Malva kit's Slint library")
slint_target_sources(barui_ui ui/menubar.slint LIBRARY_PATHS malva=${MALVA_SLINT_DIR})
target_link_libraries(barui_ui PUBLIC Slint::Slint)

# ---- The model on its own, which needs no Slint.
add_library(bar_model STATIC backend/bar_model.cpp)
target_include_directories(bar_model PUBLIC backend)
target_compile_options(bar_model PRIVATE -Wall -Wextra)
add_executable(menubar_model_test tests/model_test.cpp)
target_link_libraries(menubar_model_test PRIVATE bar_model)
enable_testing()
add_test(NAME menubar_model_test COMMAND menubar_model_test)

# ---- The stand-in backend that drives the UI.
add_library(bar_backend STATIC backend/app.cpp)
target_include_directories(bar_backend PUBLIC backend)
target_link_libraries(bar_backend PUBLIC barui_ui bar_model)
target_compile_options(bar_backend PRIVATE -Wall -Wextra)

add_executable(menubar backend/main.cpp)
target_link_libraries(menubar PRIVATE bar_backend)
target_compile_options(menubar PRIVATE -Wall -Wextra)

# ---- The window and the backend together, driven through the UI with Slint's testing backend (no
# window appears). Needs a Slint build that includes it; the SDK's does.
option(MENUBAR_APP_TEST "Build the end-to-end test (needs Slint's testing backend)" OFF)
if(MENUBAR_APP_TEST)
    add_executable(menubar_app_test tests/app_test.cpp)
    target_link_libraries(menubar_app_test PRIVATE bar_backend)
    add_test(NAME menubar_app_test COMMAND menubar_app_test)
endif()

install(TARGETS menubar RUNTIME DESTINATION bin)
