cmake_minimum_required(VERSION 3.21)
project(files-ui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Slint's C++ SDK: https://github.com/slint-ui/slint/releases (Slint-cpp-*-Linux-x86_64.tar.gz),
# found through CMAKE_PREFIX_PATH or Slint_DIR.
if(NOT Slint_FOUND) # already found when the kit builds the apps together
    find_package(Slint 1.18 REQUIRED)
endif()

# ---- The UI: ui/files.slint compiled once into a library, with its images embedded. A backend
# links it and includes "files.h" for the generated FilesWindow, FileEntry and Place.
add_library(files_ui STATIC backend/ui_anchor.cpp)
# The Malva kit's Slint library, importable as "@malva/malva.slint" (a copy of the kit sits two levels up).
set(MALVA_SLINT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../slint" CACHE PATH "The Malva kit's Slint library")
slint_target_sources(files_ui ui/files.slint LIBRARY_PATHS malva=${MALVA_SLINT_DIR})
target_link_libraries(files_ui PUBLIC Slint::Slint)

# ---- The stand-in backend that drives it.
add_library(files_backend STATIC backend/app.cpp backend/thumbnails.cpp backend/fs.cpp)
target_include_directories(files_backend PUBLIC backend)
target_link_libraries(files_backend PUBLIC files_ui)
target_compile_options(files_backend PRIVATE -Wall -Wextra)

add_executable(files backend/main.cpp)
target_link_libraries(files PRIVATE files_backend)
target_compile_options(files PRIVATE -Wall -Wextra)

# ---- The file system layer on its own, which needs no Slint.
add_library(files_fs STATIC backend/fs.cpp)
target_include_directories(files_fs PUBLIC backend)
add_executable(files_fs_test tests/fs_test.cpp)
target_link_libraries(files_fs_test PRIVATE files_fs)
enable_testing()
add_test(NAME files_fs_test COMMAND files_fs_test)

# ---- The window and the backend together, driven through the UI with Slint's testing backend (no
# window appears). Needs a Slint build that includes it; the SDK's does.
option(FILES_APP_TEST "Build the end-to-end test (needs Slint's testing backend)" OFF)
if(FILES_APP_TEST)
    add_executable(files_app_test tests/app_test.cpp)
    target_link_libraries(files_app_test PRIVATE files_backend)
    add_test(NAME files_app_test COMMAND files_app_test)
endif()

install(TARGETS files RUNTIME DESTINATION bin)
