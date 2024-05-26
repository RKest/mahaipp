cmake_minimum_required(VERSION 3.28)
project(mahai)

set(CMAKE_CXX_STANDARD 23)

find_package(pybind11 REQUIRED)
add_executable(mahai main.cpp)
target_link_libraries(mahai pybind11::embed)

include(FetchContent)
FetchContent_Declare(
        ut
        GIT_REPOSITORY https://github.com/boost-ext/ut
        GIT_TAG        v2.0.1
)
FetchContent_MakeAvailable(ut)

add_executable(test_mahai test.cpp)
target_include_directories(test_mahai PRIVATE ${ut_SOURCE_DIR}/include)

pybind11_add_module(pymahai pymod.cpp)