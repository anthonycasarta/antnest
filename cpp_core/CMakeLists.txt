cmake_minimum_required(VERSION 3.15)
project(antnest_core)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../interface)

# Set path to pybind11
set(pybind11_DIR "${PROJECT_SOURCE_DIR}/../venv/Lib/site-packages/pybind11/share/cmake/pybind11" CACHE PATH "")
set(PYTHON "${PROJECT_SOURCE_DIR}/../venv/Scripts/python.exe" CACHE PATH "")


find_package(Python REQUIRED COMPONENTS Interpreter Development)
find_package(pybind11 REQUIRED)

set(SOURCES
    folder_watcher.cpp
    backup.cpp
    bindings.cpp
    )

pybind11_add_module(antnest_core ${SOURCES})
