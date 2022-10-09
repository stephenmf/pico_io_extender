#!/usr/bin/bash
SOURCES="src io controller"
find $SOURCES \( -name "*.cpp" -or -name "*.h" \) -exec clang-format --style=Google -i {} \;
