#!/bin/bash
GLSLC=glslc
SHADER_DIR=shaders

echo "Compiling all .vert and .frag shaders in $SHADER_DIR..."

for file in "$SHADER_DIR"/*.vert; do
  echo "Compiling $(basename "$file")..."
  $GLSLC "$file" -o "$file.spv" || { echo "Failed to compile $file"; exit 1; }
done

for file in "$SHADER_DIR"/*.frag; do
  echo "Compiling $(basename "$file")..."
  $GLSLC "$file" -o "$file.spv" || { echo "Failed to compile $file"; exit 1; }
done

echo "Shader compilation successful."
