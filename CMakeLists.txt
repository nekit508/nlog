cmake_minimum_required(VERSION 3.21)
project(n2mlog)
set(CMAKE_CXX_STANDARD 20)
include(cmake/CPM.cmake)

file(GLOB_RECURSE src CONFIGURE_DEPENDS "src/*.c" "src/*.cpp")
file(GLOB_RECURSE include CONFIGURE_DEPENDS "include/*.h")
add_executable(n2mlog ${src} ${include})

CPMAddPackage(
        NAME PEGParser
        VERSION 2.1.1
        GITHUB_REPOSITORY TheLartians/PEGParser
)
target_link_libraries(n2mlog PEGParser)