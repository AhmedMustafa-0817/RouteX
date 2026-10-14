cmake_minimum_required(VERSION 3.20)

project(ROUTEX
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

# Sources used by the main application.
set(ROUTEX_SOURCES
    backend/src/Order.cpp
    backend/src/Queue.cpp
    backend/src/HashTable.cpp
    backend/src/Graph.cpp
    backend/src/MinHeap.cpp
    backend/src/Dijkstra.cpp
    backend/src/Transaction.cpp
    backend/src/TransferEvent.cpp
    backend/src/Block.cpp
    backend/src/Blockchain.cpp
    backend/src/SHA256.cpp
)

add_executable(routex
    backend/src/main.cpp
    ${ROUTEX_SOURCES}
)

target_include_directories(routex PRIVATE
    backend/include
)

# A separate executable dedicated to blockchain testing.
add_executable(test_blockchain
    backend/tests/test_blockchain.cpp
    backend/src/Block.cpp
    backend/src/TransferEvent.cpp
    backend/src/Blockchain.cpp
    backend/src/SHA256.cpp
)

target_include_directories(test_blockchain PRIVATE
    backend/include
)

add_test(
    NAME blockchain_tests
    COMMAND test_blockchain
)