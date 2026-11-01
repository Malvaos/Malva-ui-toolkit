cmake_minimum_required(VERSION 3.21)
project(dock-ui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Slint's C++ SDK: https://github.com/slint-ui/slint/releases (Slint-cpp-*-Linux-x86_64.tar.gz),
# found through CMAKE_PREFIX_PATH or Slint_DIR.
if(NOT Slint_FOUND) # already found when the kit builds the apps together
    find_package(Slint 1.18 REQUIRED)
endif()

# ---- The UI: ui/dock.slint compiled once into a library, with its images embedded. A backend
# links it and includes "dock.h" for the generated DockWindow, DockApp, MinimizedWindow and MenuEntry.
add_library(dock_ui STATIC backend/ui_anchor.cpp)
# The Malva kit's Slint library, importable as "@malva/malva.slint" (a copy of the kit sits two levels up).
set(MALVA_SLINT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../slint" CACHE PATH "The Malva kit's Slint library")
slint_target_sources(dock_ui ui/dock.slint LIBRARY_PATHS malva=${MALVA_SLINT_DIR})
target_link_libraries(dock_ui PUBLIC Slint::Slint)

# ---- The model on its own, which needs no Slint.
add_library(dock_model STATIC backend/dock_model.cpp)
target_include_directories(dock_model PUBLIC backend)
target_compile_options(dock_model PRIVATE -Wall -Wextra)
add_executable(dock_model_test tests/model_test.cpp)
target_link_libraries(dock_model_test PRIVATE dock_model)
enable_testing()
add_test(NAME dock_model_test COMMAND dock_model_test)

# ---- The stand-in backend that drives the UI.
add_library(dock_backend STATIC backend/app.cpp)
target_include_directories(dock_backend PUBLIC backend)
target_link_libraries(dock_backend PUBLIC dock_ui dock_model)
target_compile_options(dock_backend PRIVATE -Wall -Wextra)

add_executable(dock backend/main.cpp)
target_link_libraries(dock PRIVATE dock_backend)
target_compile_options(dock PRIVATE -Wall -Wextra)

# ---- The window and the backend together, driven through the UI with Slint's testing backend (no
# window appears). Needs a Slint build that includes it; the SDK's does.
option(DOCK_APP_TEST "Build the end-to-end test (needs Slint's testing backend)" OFF)
if(DOCK_APP_TEST)
    add_executable(dock_app_test tests/app_test.cpp)
    target_link_libraries(dock_app_test PRIVATE dock_backend)
    add_test(NAME dock_app_test COMMAND dock_app_test)
endif()

install(TARGETS dock RUNTIME DESTINATION bin)
