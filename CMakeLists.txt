cmake_minimum_required(VERSION 3.16)
project(MiniDB-Engine LANGUAGES CXX)

add_executable(minidb
    src/main.cpp
    src/tokenizer.cpp
    src/parser.cpp
    src/database.cpp
)

target_include_directories(minidb PRIVATE include)
target_compile_features(minidb PRIVATE cxx_std_17)
