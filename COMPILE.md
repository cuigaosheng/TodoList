# TodoList Compile Guide

## Requirements
- CMake 3.16+
- MinGW 13.1.0 (from Qt Tools)
- Qt 6.11.0

## Build Steps

1. cd C:/Users/cuiga/TodoList/build-win

2. cmake configure:
cmake -G "MinGW Makefiles" -DCMAKE_C_COMPILER=C:/Qt/Tools/mingw1310_64/bin/gcc.exe -DCMAKE_CXX_COMPILER=C:/Qt/Tools/mingw1310_64/bin/g++.exe -DCMAKE_MAKE_PROGRAM=C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe -DCMAKE_PREFIX_PATH=C:/Qt/6.11.0/mingw_64 ..

3. cmake --build .

## Common Issues
1. MinGW make not found - use -DCMAKE_MAKE_PROGRAM with full path
2. Qt6 not found - add -DCMAKE_PREFIX_PATH=C:/Qt/6.11.0/mingw_64
3. Duplicate declarations in mainwindow.h - remove duplicate lines
4. exe locked - taskkill /IM TodoList.exe /F
