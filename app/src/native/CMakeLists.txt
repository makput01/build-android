cmake_minimum_required(VERSION 3.22.1)

set(LOCAL_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(MODULE_NAME main)

project(${MODULE_NAME})

set(CMAKE_SYSTEM_NAME Android)
set(CMAKE_ANDROID_API 26)
set(CMAKE_SYSTEM_VERSION 26)
set(CMAKE_ANDROID_STL_TYPE c++_static)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE MinSizeRel CACHE STRING "Build type" FORCE)
endif()

file(GLOB IMGUI_SRC
        "${LOCAL_PATH}/ImGui/*.c"
        "${LOCAL_PATH}/ImGui/*.cpp"
)

file(GLOB KITTYMEMORY_SRC
        "${LOCAL_PATH}/Includes/KittyMemory/*.c"
        "${LOCAL_PATH}/Includes/KittyMemory/*.cpp"
)

add_library(libkeystone STATIC IMPORTED)
set_target_properties(libkeystone PROPERTIES
        IMPORTED_LOCATION
        "${LOCAL_PATH}/Includes/KittyMemory/Deps/Keystone/libs-android/${ANDROID_ABI}/libkeystone.a"
)

add_library(${MODULE_NAME} SHARED
        native-lib.cpp
        ${IMGUI_SRC}
        ImGui/backends/imgui_impl_android.cpp
        ImGui/backends/imgui_impl_opengl3.cpp
        ${KITTYMEMORY_SRC}
)

set(OPTIMIZ_FLAGS
        -Oz
        -flto
        -fPIC
        -fvisibility=hidden
        -fvisibility-inlines-hidden
)

target_include_directories(${MODULE_NAME} PRIVATE
        ${LOCAL_PATH}
        ${LOCAL_PATH}/ImGui
)

target_compile_definitions(${MODULE_NAME} PRIVATE
        NDEBUG
        _FORTIFY_SOURCE=2
)

target_compile_options(${MODULE_NAME} PRIVATE
        ${ARCH_CFLAGS}
        ${OPTIMIZ_FLAGS}
        -g0
        -ffunction-sections
        -fdata-sections
        -fstack-protector-strong
        -fno-rtti
        -fno-exceptions
        -fno-ident
        -fno-asynchronous-unwind-tables
        -fno-unwind-tables
        -fomit-frame-pointer
        -fmerge-all-constants
        -fno-strict-aliasing
        -fno-math-errno
        -w
)

target_link_options(${MODULE_NAME} PRIVATE
        ${ARCH_CFLAGS}
        ${OPTIMIZ_FLAGS}
        -g0
        -Wl,--gc-sections
        -Wl,--strip-debug
        -Wl,--strip-all
        -Wl,--exclude-libs,ALL
        -Wl,--icf=safe
        -Wl,--hash-style=gnu
        -Wl,--build-id=none
        -Wl,-z,relro
        -Wl,-z,now
        -Wl,-z,noexecstack
        -pthread
)

target_link_libraries(${MODULE_NAME}
        libkeystone
        log
        android
        EGL
        GLESv2
        GLESv3
        GLESv1_CM
        z
)
