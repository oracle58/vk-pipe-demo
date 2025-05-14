@echo off
set GLSLC=glslc
set SHADER_DIR=shaders

echo Compiling all .vert and .frag shaders in %SHADER_DIR%...

for %%F in (%SHADER_DIR%\*.vert) do (
    echo Compiling %%~nxF...
    %GLSLC% "%%F" -o "%%F.spv"
    if %errorlevel% neq 0 (
        echo Failed to compile vertex shader %%~nxF
        exit /b %errorlevel%
    )
)

for %%F in (%SHADER_DIR%\*.frag) do (
    echo Compiling %%~nxF...
    %GLSLC% "%%F" -o "%%F.spv"
    if %errorlevel% neq 0 (
        echo Failed to compile fragment shader %%~nxF
        exit /b %errorlevel%
    )
)

echo Shader compilation successful.
