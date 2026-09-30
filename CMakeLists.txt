cmake_minimum_required(VERSION 3.20)

project(ROUTEX
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(backend/include)

set(SOURCES
    backend/src/main.cpp
    backend/src/Order.cpp
    backend/src/Queue.cpp
    backend/src/HashTable.cpp
    backend/src/Graph.cpp
    backend/src/MinHeap.cpp
    backend/src/Dijkstra.cpp
    backend/src/Transaction.cpp
    backend/src/Block.cpp
    backend/src/Blockchain.cpp
    backend/src/SHA256.cpp
)

add_executable(routex ${SOURCES})