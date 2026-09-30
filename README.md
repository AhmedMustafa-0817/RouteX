# ROUTEX

## DSA-Based Supply & Shipment Management System

ROUTEX is a C++-based logistics management system designed as a **Data Structures and Algorithms course project**.

The system manages delivery orders, processes them using a queue, provides fast shipment lookup through hashing, represents transportation networks as weighted graphs, calculates the fastest city-to-city route using Dijkstra's algorithm with a min-heap, and maintains a tamper-evident shipment-event history using a blockchain implemented with linked blocks and SHA-256 hashing.

---

# 1. Project Purpose

ROUTEX addresses a simplified logistics problem:

A logistics organization manages multiple delivery orders moving between warehouses and cities. The system needs to:

1. Organize incoming orders.
2. Process orders in a defined order.
3. Find shipments quickly.
4. Represent the transportation network.
5. Calculate the fastest route between two cities.
6. Maintain a historical record of important shipment events.
7. Detect unauthorized modification of previously recorded blockchain data.

The project is primarily focused on applying **Data Structures and Algorithms to a real-world logistics scenario**.

Blockchain is used as a supporting integrity mechanism rather than as the entire purpose of the project.

---

# 2. Core Concepts

ROUTEX uses the following major DSA concepts.

| Concept              | Purpose in ROUTEX                               |
| -------------------- | ----------------------------------------------- |
| Queue                | FIFO management of delivery orders              |
| Hash Table           | Fast shipment lookup by Shipment ID             |
| Graph                | Representation of cities, warehouses and routes |
| Min-Heap             | Priority structure used by Dijkstra             |
| Dijkstra's Algorithm | Fastest route calculation                       |
| Linked List          | Structure connecting blockchain blocks          |
| Hashing              | Fast lookup through the hash table              |
| SHA-256              | Cryptographic hashing for blockchain integrity  |
| File Handling        | Saving/loading project data                     |
| Big-O Analysis       | Evaluating algorithm efficiency                 |

---

# 3. System Architecture

```text
                         ROUTEX
                            |
          ┌─────────────────┼─────────────────┐
          |                 |                 |
       ORDERS             ROUTES           HISTORY
          |                 |                 |
       Queue              Graph          Blockchain
          |                 |                 |
     Hash Table         Min-Heap          Linked List
          |                 |                 |
   Shipment Search      Dijkstra          SHA-256
                            |
                      Fastest Route
```

---

# 4. Main Workflow

```text
Create Shipment
      |
      ↓
Add to Queue
      |
      ↓
Process Next Shipment
      |
      ├───────────────┐
      ↓               ↓
Hash Table        Blockchain
      |               |
Find Shipment    Record Event
                      |
                      ↓
                 Create Block
                      |
                      ↓
                  SHA-256
```

For route planning:

```text
Source City
     |
     ↓
Weighted Graph
     |
     ↓
Min-Heap
     |
     ↓
Dijkstra
     |
     ↓
Fastest Route
     |
     ↓
Destination City
```

---

# 5. Blockchain Design

ROUTEX uses a small educational blockchain.

The blockchain records shipment events such as:

* Shipment created
* Shipment dispatched
* Shipment arrived at a warehouse
* Shipment transferred to another warehouse
* Shipment delivered

A block can contain multiple transactions.

Example:

```text
Block 3
---------------------------------
Transaction 1:
SH101 CREATED - Karachi

Transaction 2:
SH101 DISPATCHED - Karachi

Transaction 3:
SH102 CREATED - Multan

Previous Hash:
ABC123

Current Hash:
XYZ789
---------------------------------
```

Blocks are linked:

```text
Genesis
   ↓
Block 1
   ↓
Block 2
   ↓
Block 3
```

Each block stores:

* Block index
* Timestamp
* Transactions
* Previous hash
* Current hash

---

# 6. Why Blockchain Is Used

Blockchain is used to make the recorded shipment history **tamper-evident**.

Suppose an original record says:

```text
SH101 arrived at Multan on Monday.
```

If somebody later changes it to:

```text
SH101 arrived at Multan on Tuesday.
```

the block's data changes.

The SHA-256 hash is recalculated.

If:

```text
Stored Hash != Recalculated Hash
```

the system reports:

```text
TAMPERING DETECTED
```

The blockchain therefore provides integrity checking for previously recorded events.

---

# 7. Important Limitation

Blockchain does not automatically determine whether the information entered into the system was truthful.

For example, if someone enters a false event correctly the first time, the blockchain does not know that the event was false.

The blockchain's purpose in ROUTEX is primarily:

> Detecting unauthorized modification of recorded historical data.

---

# 8. Hashing vs SHA-256

ROUTEX uses hashing in two different ways.

## Hash Table Hashing

Used to find a shipment efficiently.

```text
Shipment ID
     |
  Hash Function
     |
   Index
     |
 Shipment
```

## SHA-256

Used for blockchain integrity.

```text
Block Data
    |
 SHA-256
    |
Hash
```

These are different uses of hashing.

---

# 9. Fastest Route Calculation

The logistics network is represented as a weighted graph.

### Vertices

Cities or warehouses.

Example:

```text
Karachi
Hyderabad
Sukkur
Multan
Lahore
Islamabad
```

### Edges

Transport routes.

### Edge Weight

Travel time.

For example:

```text
Karachi → Hyderabad = 3 hours
Hyderabad → Sukkur = 5 hours
Sukkur → Multan = 4 hours
```

Because the weights represent travel time, the system calculates the **fastest route**, not simply the route with the fewest cities.

Dijkstra's algorithm is used to determine the minimum total travel time.

---

# 10. Order Queue

Orders are processed using FIFO order.

Example:

```text
FRONT
  |
  ↓
SH101 → SH102 → SH103 → SH104
                              ↑
                             REAR
```

The first order added is processed first.

---

# 11. Project Folder Structure

```text
routex-dsa/
│
├── backend/
│   ├── include/
│   │   ├── Order.h
│   │   ├── Queue.h
│   │   ├── HashTable.h
│   │   ├── Graph.h
│   │   ├── MinHeap.h
│   │   ├── Dijkstra.h
│   │   ├── Transaction.h
│   │   ├── Block.h
│   │   ├── Blockchain.h
│   │   └── SHA256.h
│   │
│   ├── src/
│   │   ├── main.cpp
│   │   ├── Order.cpp
│   │   ├── Queue.cpp
│   │   ├── HashTable.cpp
│   │   ├── Graph.cpp
│   │   ├── MinHeap.cpp
│   │   ├── Dijkstra.cpp
│   │   ├── Transaction.cpp
│   │   ├── Block.cpp
│   │   ├── Blockchain.cpp
│   │   └── SHA256.cpp
│   │
│   └── server/
│       └── Server.cpp
│
├── frontend/
│   ├── index.html
│   ├── css/
│   │   └── style.css
│   └── js/
│       ├── app.js
│       ├── orders.js
│       ├── routes.js
│       └── blockchain.js
│
├── data/
│   ├── orders.csv
│   └── routes.csv
│
├── tests/
│   ├── test_queue.cpp
│   ├── test_hash_table.cpp
│   ├── test_graph.cpp
│   ├── test_dijkstra.cpp
│   └── test_blockchain.cpp
│
├── docs/
│   ├── ARCHITECTURE.md
│   ├── COMPLEXITY_ANALYSIS.md
│   └── TEAM_WORKFLOW.md
│
├── .github/
│   └── pull_request_template.md
│
├── .gitignore
├── CMakeLists.txt
├── CONTRIBUTING.md
└── README.md
```

---

# 12. Development Environment

## Windows

Recommended setup:

* Git
* Visual Studio
* Desktop development with C++
* CMake

The project is built using CMake.

Typical commands:

```powershell
git clone YOUR_REPOSITORY_URL
cd routex-dsa

git fetch origin
git switch develop

cmake -B build
cmake --build build
```

For Visual Studio multi-configuration builds, the executable may be placed under a configuration directory such as:

```text
build/Debug/
```

Run the resulting executable from the appropriate configuration folder.

---

# 13. Mac

Recommended setup:

* Xcode Command Line Tools
* Clang
* CMake
* Git
* Visual Studio Code
* GitHub CLI

Check Git:

```bash
git --version
```

Check compiler:

```bash
clang++ --version
```

Check CMake:

```bash
cmake --version
```

Authenticate GitHub CLI:

```bash
gh auth login
```

Configure Git:

```bash
gh auth setup-git
```

Clone:

```bash
git clone YOUR_REPOSITORY_URL
```

Build:

```bash
cmake -B build
cmake --build build
```

Run:

```bash
./build/routex
```

---

# 14. CMake

ROUTEX uses CMake to keep the build process consistent across different operating systems.

The normal build process is:

```text
Source Code
    |
    ↓
CMake
    |
    ↓
build/
    |
    ↓
Executable
```

The `build/` directory is generated automatically and should not be committed to GitHub.

---

# 15. Git Branch Strategy

The main branches are:

```text
main
develop
```

### main

Stable and submission-ready code.

### develop

Integrated development version.

### Feature branches

Individual tasks are implemented separately.

Examples:

```text
feature/order-queue
feature/hash-table
feature/graph
feature/min-heap
feature/dijkstra
feature/blockchain-core
feature/sha256
feature/blockchain-validation
feature/frontend
feature/testing
```

---

# 16. Git Workflow

Always start by updating `develop`.

```bash
git switch develop
git pull origin develop
```

Create your feature branch:

```bash
git switch -c feature/your-feature
```

Work on the feature.

Check changes:

```bash
git status
```

Stage:

```bash
git add .
```

Commit:

```bash
git commit -m "Describe the change"
```

Push:

```bash
git push -u origin feature/your-feature
```

Then open a Pull Request:

```text
feature/your-feature
        ↓
     develop
```

The feature should be reviewed before being merged.

---

# 17. Pull Request Rules

Every Pull Request should:

1. Have a meaningful title.
2. Explain what changed.
3. Explain what was tested.
4. Compile successfully.
5. Avoid unrelated changes.
6. Be reviewed by another team member.

Example title:

```text
Implement FIFO shipment queue
```

Example description:

```text
Implemented the Order and Queue classes.

Added:
- enqueue()
- dequeue()
- front()
- isEmpty()
- display()

Tested:
- Empty queue
- One order
- Multiple orders
```

---

# 18. Commit Message Examples

Good:

```text
Initialize ROUTEX project structure
Implement FIFO shipment queue
Implement separate chaining hash table
Add weighted logistics graph
Implement min heap
Implement Dijkstra route calculation
Implement blockchain block structure
Add SHA-256 hashing
Add blockchain validation
Connect shipment events to blockchain
```

Avoid vague messages such as:

```text
Changes
Update
Final
Done
Stuff
```

---

# 19. Coding Rules

## Rule 1

Do not directly develop on `main`.

## Rule 2

Do not commit generated build files.

## Rule 3

Do not commit passwords, tokens, or private credentials.

## Rule 4

One feature should normally correspond to one feature branch.

## Rule 5

Keep `.h` declarations separate from `.cpp` implementations.

## Rule 6

Do not place the entire project inside `main.cpp`.

## Rule 7

Test every DSA implementation.

## Rule 8

Do not change another member's code without discussing the integration.

## Rule 9

Pull the latest `develop` before starting new work.

## Rule 10

A broken feature should not be merged simply to finish the day.

---

# 20. Team

Project:

**ROUTEX**

Team members:

```text
Ahmed Mustafa
Jawwad
Hania
```

Daily rotation:

```text
Day 1  Ahmed
Day 2  Jawwad
Day 3  Hania

Day 4  Ahmed
Day 5  Jawwad
Day 6  Hania

Day 7  Ahmed
Day 8  Jawwad
Day 9  Hania

Day 10 Ahmed
Day 11 Jawwad
Day 12 Hania

Day 13 Ahmed
Day 14 Jawwad
Day 15 Hania
```

The owner of a day is responsible for completing the assigned task and providing a clear handoff.

---

# 21. Build Order

ROUTEX should be built in this order:

```text
1. Order
   ↓
2. Queue
   ↓
3. Hash Table
   ↓
4. Graph
   ↓
5. Min-Heap
   ↓
6. Dijkstra
   ↓
7. Blockchain
   ↓
8. SHA-256
   ↓
9. Blockchain Validation
   ↓
10. System Integration
   ↓
11. Persistence
   ↓
12. Backend Interface
   ↓
13. Frontend
   ↓
14. Testing
   ↓
15. Final Integration
```

This order is intentional.

---

# 22. Testing Requirements

Every major DSA component must have tests.

### Queue

Test:

* Empty queue
* One element
* Multiple elements
* Dequeue from empty queue
* FIFO order

### Hash Table

Test:

* Insert
* Search
* Missing shipment
* Duplicate shipment ID
* Collision handling

### Graph

Test:

* Add city
* Add route
* Multiple connections
* Invalid city

### Dijkstra

Test:

* Normal route
* Multiple possible routes
* Same source and destination
* Unreachable destination

### Blockchain

Test:

* Genesis block
* Adding blocks
* Hash generation
* Previous hash linkage
* Valid chain
* Tampered chain

---

# 23. Final Demonstration

The final presentation should demonstrate one complete shipment scenario.

Example:

```text
Shipment ID:
SH101

Source:
Karachi

Destination:
Lahore
```

Add several orders:

```text
SH101
SH102
SH103
SH104
```

Show:

```text
Queue:
SH101 → SH102 → SH103 → SH104
```

Process the first order.

Show shipment lookup:

```text
Search SH101
        ↓
Hash Table
```

Record event:

```text
SH101 CREATED
```

Add the transaction to the blockchain.

Calculate route:

```text
Karachi → Lahore
        ↓
Dijkstra
        ↓
Fastest Route
```

Verify blockchain:

```text
BLOCKCHAIN VALID
```

Then demonstrate tampering on a test record.

Change:

```text
Monday → Tuesday
```

Run verification.

Expected result:

```text
TAMPERING DETECTED
```

---

# 24. Out of Scope

The following are intentionally not part of ROUTEX:

* Machine Learning
* GPS tracking
* Cryptocurrency
* Mining
* Smart contracts
* Payment processing
* Complex authentication
* AI route prediction
* Mobile application
* External logistics APIs
* Real government or commercial transport databases

The purpose is to keep the project focused on DSA, algorithms, software structure, and a small supporting blockchain component.

---

# 25. Learning Outcomes

By completing ROUTEX, the team should understand:

* Practical use of queues.
* Practical use of hash tables.
* Collision handling.
* Graph representation.
* Priority queues and heaps.
* Dijkstra's algorithm.
* Linked-list implementation.
* Hashing.
* SHA-256.
* Blockchain data structure.
* Blockchain integrity validation.
* File handling.
* C++ modular design.
* CMake.
* Git and GitHub collaboration.
* Branching and Pull Requests.
* Basic frontend/backend integration.
* Algorithmic complexity.

---

# 26. Final Goal

ROUTEX should demonstrate that Data Structures and Algorithms are not only theoretical topics.

The project shows how:

```text
Queue
    ↓
Order Management

Hash Table
    ↓
Fast Shipment Search

Graph
    ↓
Transportation Network

Min-Heap
    ↓
Efficient Priority Selection

Dijkstra
    ↓
Fastest Route

Linked List
    ↓
Blockchain Structure

SHA-256
    ↓
Integrity Verification
```

can work together inside one practical logistics system.
