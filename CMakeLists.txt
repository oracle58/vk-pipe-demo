cmake_minimum_required(VERSION 3.20)
project(vk-pipe-demo LANGUAGES CXX C)

# --------------- GLOBALS -----------------------------------------------------
set(CMAKE_CXX_STANDARD 20)

# ───── Tell MSVC to read & write files as UTF‑8 ──────────────────────────────
if (MSVC)
    add_compile_options(/utf-8)
endif()

# --------------- SUB‑PROJECTS ------------------------------------------------
add_subdirectory(src)
#add_subdirectory(shaders)






