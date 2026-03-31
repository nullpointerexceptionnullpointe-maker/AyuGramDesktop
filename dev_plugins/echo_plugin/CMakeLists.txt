cmake_minimum_required(VERSION 3.16)
project(echo_plugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(echo_plugin SHARED echo_plugin.cpp)
target_include_directories(echo_plugin PRIVATE
    /content/tdesktop/Telegram/SourceFiles/plugins
)
set_target_properties(echo_plugin PROPERTIES
    PREFIX ""
    OUTPUT_NAME "echo_plugin"
)
