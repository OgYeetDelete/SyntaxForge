cmake_minimum_required(VERSION 4.3)
project(CompilerDesign)

set(CMAKE_CXX_STANDARD 20)

add_executable(CompilerDesign
        CompilerDesignPhase1.cpp
        syntaxforge.cpp)

add_executable(CompilerP2R2 syntaxforgephase2.cpp)
