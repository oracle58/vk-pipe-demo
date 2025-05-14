# Shader compilation script
if(WIN32)
    add_custom_target(compile_shaders
        COMMAND ${CMAKE_COMMAND} -E echo "Compiling shaders (Windows)..."
        COMMAND ${CMAKE_COMMAND} -E env "PATH=$ENV{PATH}" ${CMAKE_SOURCE_DIR}/shaders/compile_shaders.bat
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Compiling GLSL shaders with .bat"
    )
else()
    add_custom_target(compile_shaders
        COMMAND ${CMAKE_COMMAND} -E echo "Compiling shaders (Unix)..."
        COMMAND bash ${CMAKE_SOURCE_DIR}/shaders/compile_shaders.sh
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Compiling GLSL shaders with .sh"
    )
endif()