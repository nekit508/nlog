cmake_minimum_required(VERSION 3.21)
project(n2mlog)

set(CMAKE_CXX_STANDARD 20)

include(cmake/CPM.cmake)

CPMAddPackage(
        NAME PEGParser
        VERSION 2.1.1
        GITHUB_REPOSITORY TheLartians/PEGParser
)

add_executable(n2mlog src/main.cpp include/utils.hpp
        include/ast.hpp
        src/Scope.cpp
        include/Scope.h
        src/Command.cpp
        include/Command.h
        src/Var.cpp
        include/Var.h)

target_link_libraries(n2mlog PEGParser::PEGParser)
