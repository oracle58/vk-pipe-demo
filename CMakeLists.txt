cmake_minimum_required(VERSION 3.20)
project(vk-pipe-demo LANGUAGES CXX C)

# --------------- OPTIONS -----------------------------------------------------
option(ENGINE_SHARED "Build engine as a DLL" ON)

# --------------- GLOBALS -----------------------------------------------------
set(CMAKE_CXX_STANDARD 20)

# ───── Tell MSVC to read & write files as UTF‑8 ──────────────────────────────
if (MSVC)
    add_compile_options(/utf-8)
endif()

# ─────────────────── Sources ────────────────────────────────────────────────
file(GLOB_RECURSE ENGINE_SRC CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/engine/*.cpp" "${CMAKE_SOURCE_DIR}/engine/*.h")

if (ENGINE_SHARED)
    add_library(engine SHARED ${ENGINE_SRC})
else()
    add_library(engine STATIC ${ENGINE_SRC})
endif()

set_target_properties(engine PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
        POSITION_INDEPENDENT_CODE ON)

target_compile_definitions(engine
    PUBLIC $<$<BOOL:ENGINE_SHARED>:ENGINE_SHARED>
)

target_include_directories(engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

# ─────────────────── Dependencies ───────────────────────────────────────────
find_package(glfw3 CONFIG REQUIRED)
find_package(glm   CONFIG REQUIRED)
find_package(fmt   CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(volk  CONFIG QUIET)
find_package(imgui CONFIG REQUIRED)

target_link_libraries(engine
    PUBLIC
    glfw
    glm::glm
    fmt::fmt
    Vulkan::Vulkan
    $<$<TARGET_EXISTS:volk::volk>:volk::volk>
    imgui::imgui
)

# ──────────── per‑config output folders for engine ──────────────────────────
foreach(cfg IN ITEMS debug release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${cfg}" CFG_UPPER)
    set_target_properties(engine PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${CFG_UPPER}       "${CMAKE_SOURCE_DIR}/bin/${cfg}"
        LIBRARY_OUTPUT_DIRECTORY_${CFG_UPPER}       "${CMAKE_SOURCE_DIR}/bin/${cfg}"
        ARCHIVE_OUTPUT_DIRECTORY_${CFG_UPPER}       "${CMAKE_SOURCE_DIR}/bin/intermediate/${cfg}"
    )
endforeach()

# ──────────── Executable ────────────────────────────────────────────────────
add_executable(demo "main.cpp")
target_link_libraries(demo PRIVATE engine)
target_include_directories(demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ──────────── delay‑load the engine DLL for hot‑reload (Windows only) ───────
if (WIN32 AND ENGINE_SHARED)
    set_property(TARGET demo APPEND PROPERTY LINK_OPTIONS "/DELAYLOAD:engine.dll")
endif()

# ──────────── per‑config output folders for demo ──────────────────────────
foreach(cfg IN ITEMS debug release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${cfg}" CFG_UPPER)
    set_target_properties(demo PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${CFG_UPPER}       "${CMAKE_SOURCE_DIR}/bin/${cfg}"
        LIBRARY_OUTPUT_DIRECTORY_${CFG_UPPER}       "${CMAKE_SOURCE_DIR}/bin/${cfg}"
        ARCHIVE_OUTPUT_DIRECTORY_${CFG_UPPER}       "${CMAKE_SOURCE_DIR}/bin/intermediate/${cfg}"
    )
endforeach()

# ──────────── copy DLLs next to the exe -------------------------------------

# 1. copy the engine itself (explicit, harmless if duplicated by step 2)
if (WIN32 AND ENGINE_SHARED)
    add_custom_command(TARGET demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:engine>
                $<TARGET_FILE_DIR:demo>)
endif()

# 2. copy *all* runtime DLL dependencies (transitive vcpkg DLLs, etc.)
add_custom_command(TARGET demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:demo>
            $<TARGET_FILE_DIR:demo>
    COMMAND_EXPAND_LISTS
)

# 3. copy Vulkan runtime DLL if found (Windows only)
if (WIN32)
    find_path(VULKAN_BIN_PATH NAMES vulkan-1.dll PATHS
        "$ENV{VULKAN_SDK}/Bin"
        "$ENV{VULKAN_SDK}/Bin32"
        "$ENV{VULKAN_SDK}/Bin64"
        "${CMAKE_SOURCE_DIR}/external/vulkan/Bin"
        "${CMAKE_SOURCE_DIR}/external/vulkan/Bin32"
        "${CMAKE_SOURCE_DIR}/external/vulkan/Bin64"
        NO_DEFAULT_PATH
    )
    if (VULKAN_BIN_PATH)
        add_custom_command(TARGET demo POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${VULKAN_BIN_PATH}/vulkan-1.dll"
                $<TARGET_FILE_DIR:demo>
            COMMENT "Copying vulkan-1.dll to output directory"
        )
    endif()
endif()

# ──────────── copy all *.spv shaders into output dir ───────────────
file(GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/shaders/*.spv")

foreach(cfg IN ITEMS debug release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${cfg}" CFG_UPPER)
    add_custom_command(TARGET demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:demo>/shaders"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${SHADER_FILES}
            "$<TARGET_FILE_DIR:demo>/shaders"
        COMMENT "Copying all .spv shaders to output directory (${cfg})"
    )
endforeach()