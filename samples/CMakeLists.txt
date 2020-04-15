cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(../include)

add_executable(sample_syntax_analysis labs/sample_syntax_analysis.cpp)

add_executable(sample_directed_graph utils/sample_directed_graph.cpp)
add_executable(sample_smart_ifstream utils/sample_smart_ifstream.cpp)
add_executable(sample_automaton utils/sample_automaton.cpp)
