# MiniDB-Engine

A small C++ SQL tokenizer project.

## Build

```sh
g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude src/main.cpp src/tokenizer.cpp src/parser.cpp src/database.cpp -o minidb
.\minidb.exe
```

The same sources can also be built with CMake:

```sh
cmake -S . -B build
cmake --build build
```