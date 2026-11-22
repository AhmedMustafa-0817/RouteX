cmake_minimum_required(VERSION 3.20)

project(ROUTEX VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

set(ROUTEX_SOURCES
    backend/src/Order.cpp
    backend/src/Shipment.cpp
    backend/src/Storage.cpp
    backend/src/ShipmentManager.cpp
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

target_include_directories(routex PRIVATE backend/include)

# Every tests/<name>.cpp becomes an executable registered with CTest.
# Run them all with: ctest --test-dir build --output-on-failure
set(ROUTEX_TESTS
    test_queue
    test_hash_table
    test_graph
    test_minheap
    test_dijkstra
    test_SHA256
    test_transfer_event
    test_block
    test_blockchain
    test_storage
)

foreach(test_name IN LISTS ROUTEX_TESTS)
    add_executable(${test_name} tests/${test_name}.cpp ${ROUTEX_SOURCES})
    # backend/src is also listed because some tests include "../include/X.h".
    target_include_directories(${test_name} PRIVATE backend/include backend/src)
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()

add_executable(routex_api
    backend/server/Server.cpp
    ${ROUTEX_SOURCES}
)

target_include_directories(routex_api PRIVATE backend/include)

if(WIN32)
    target_link_libraries(routex_api PRIVATE ws2_32)
endif()