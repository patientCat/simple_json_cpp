cmake_minimum_required(VERSION 3.16)
project(simple_json_cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(.)
# 添加boost头文件搜索路径
include_directories(/usr/local/lib/)

# 添加boost库文件搜索路径
link_directories(/usr/local/lib/boost_lib/)

add_executable(simple_json_cpp
        JsonParse.hh
        JsonParse.cc
        main.cpp)
