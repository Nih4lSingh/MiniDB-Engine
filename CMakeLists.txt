cmake_minimum_required(VERSION 3.16)
project(MiniDB-Engine LANGUAGES CXX)

add_executable(minidb
    src/main.cpp
    src/tokenizer.cpp
)

target_include_directories(minidb PRIVATE include)
target_compile_features(minidb PRIVATE cxx_std_17)
