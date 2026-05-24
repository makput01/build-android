cmake_minimum_required(VERSION 3.22.1)

project("imgui")

# Tentukan lokasi ImGui
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

# Tambahkan file sumber ImGui
add_library(
    imgui
    STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
)

# Tambahkan implementasi Android + OpenGL3
add_library(
    loader
    SHARED
    native-lib.cpp
    imgui/backends/imgui_impl_android.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

# Sertakan direktori header
target_include_directories(loader PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# Tautkan perpustakaan
target_link_libraries(
    loader
    imgui
    android
    EGL
    GLESv3
    log
)

