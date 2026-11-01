# The three desktop apps, ported onto the kit: the Files app, the dock and the menu bar. Each is also a project of its
# own (cmake -S apps/dock ...), with the same Slint requirement; this file builds them together, with their
# end-to-end tests on, when the kit is configured with -DMALVA_BUILD_APPS=ON.
foreach(opt FILES_APP_TEST DOCK_APP_TEST MENUBAR_APP_TEST)
    if(NOT DEFINED ${opt})
        set(${opt} ON CACHE BOOL "Build the end-to-end test (needs Slint's testing backend)")
    endif()
endforeach()
add_subdirectory(files)
add_subdirectory(dock)
add_subdirectory(menubar)
