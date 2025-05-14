cmake_minimum_required(VERSION 3.20)
project(vk-pipe-demo LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)

# ───── Tell MSVC to read & write files as UTF‑8 ──────────────────────────────
if (MSVC)
    add_compile_options(/utf-8)
endif()

add_subdirectory(src)
add_subdirectory(shaders)

# ───── Generate Binary ──────────────────────────────

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(vk-pipe-demo
    main.cpp
    ${SRC_FILES}
)

target_include_directories(vk-pipe-demo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(vk-pipe-demo PRIVATE src)

# Copy only the required shader files to the binary output directory after build
add_custom_command(TARGET vk-pipe-demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:vk-pipe-demo>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/shaders/sb_shader.vert.spv"
        "$<TARGET_FILE_DIR:vk-pipe-demo>/shaders/sb_shader.vert.spv"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/shaders/sb_shader.frag.spv"
        "$<TARGET_FILE_DIR:vk-pipe-demo>/shaders/sb_shader.frag.spv"
)

# Set per-config output directories
foreach(cfg IN ITEMS Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${cfg}" CFG_UPPER)
    set_target_properties(vk-pipe-demo PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${CFG_UPPER} "${CMAKE_CURRENT_SOURCE_DIR}/bin/${cfg}"
        LIBRARY_OUTPUT_DIRECTORY_${CFG_UPPER} "${CMAKE_CURRENT_SOURCE_DIR}/bin/${cfg}"
        ARCHIVE_OUTPUT_DIRECTORY_${CFG_UPPER} "${CMAKE_CURRENT_SOURCE_DIR}/bin/intermediate/${cfg}"
    )
endforeach()



