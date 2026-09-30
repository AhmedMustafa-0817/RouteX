# ROUTEX

### DSA-Based Supply & Shipment Management System

ROUTEX is a C++ supply and shipment management system built as a Data Structures and Algorithms course project.

The system demonstrates how different data structures and algorithms can be combined to manage orders, calculate efficient shipment routes, and maintain a history of transactions.

---

## Core Features

### 📦 Order Management

Orders are stored and managed using:

* Queue
* Hash Table

The queue handles order processing, while the hash table allows fast shipment/order searching.

### 🗺️ Route Management

The transportation network is represented using a graph.

ROUTEX uses:

* Graph
* Min Heap
* Dijkstra's Algorithm

This allows the system to determine the shortest/fastest route between cities.

### 🔗 Transaction History

Shipment transactions are recorded using a blockchain-style structure.

This component uses:

* Linked List concepts
* Blocks
* Transactions
* SHA-256 hashing

The purpose is to demonstrate data integrity and chained transaction history.

---

## Data Structures & Algorithms

| Component           | Data Structure / Algorithm |
| ------------------- | -------------------------- |
| Order Processing    | Queue                      |
| Order Search        | Hash Table                 |
| City Network        | Graph                      |
| Route Optimization  | Dijkstra's Algorithm       |
| Priority Management | Min Heap                   |
| Transaction History | Linked List / Blockchain   |
| Data Integrity      | SHA-256                    |

---

## Project Structure

```text
routex-dsa/
│
├── backend/
│   ├── include/          # Header files
│   ├── src/              # C++ implementations
│   └── server/           # Backend/server code
│
├── frontend/
│   ├── index.html
│   ├── css/
│   └── js/
│
├── data/                 # Input datasets
│
├── tests/                # Data structure and algorithm tests
│
├── docs/                 # Project documentation
│
├── .github/
│   └── pull_request_template.md
│
├── CMakeLists.txt
├── CONTRIBUTING.md
├── README.md
└── .gitignore
```

---

## Requirements

Before building ROUTEX, make sure the following are installed:

* Git
* C++ compiler with C++17 support
* CMake 3.20 or newer

macOS users can use Apple's Clang compiler.

---

## Building the Project

Clone the repository:

```bash
git clone https://github.com/AhmedMustafa-0817/RouteX.git
cd routex-dsa
```

Switch to the development branch:

```bash
git switch develop
```

Create a build directory:

```bash
mkdir build
cd build
```

Configure the project:

```bash
cmake ..
```

Build:

```bash
cmake --build .
```

The exact executable name may change as development continues.

---

## Development Workflow

The `main` branch contains the stable version of the project.

The `develop` branch contains the team's current working version.

Individual features should be developed using feature branches.

Example:

```bash
git switch develop
git pull origin develop

git switch -c feature/order-queue
```

After completing the feature:

```bash
git add .
git commit -m "Implement order queue"

git push -u origin feature/order-queue
```

Then open a Pull Request targeting:

```text
develop
```

Do not directly push feature work to `main`.

---

## Team

### Developers

* Ahmed Mustafa
* Jawwad
* Hania

Each team member should work using their own GitHub account.

---

## Project Scope

ROUTEX intentionally focuses on Data Structures and Algorithms.

The project does **not** include:

* Machine Learning
* GPS tracking
* Online payments
* Authentication systems
* Cryptocurrency
* Smart contracts
* AI prediction

The goal is to demonstrate practical use of DSA rather than adding unrelated technologies.

---

## Documentation

Additional project documentation will be maintained in:

```text
docs/
```

Important documents include:

* `ARCHITECTURE.md`
* `COMPLEXITY_ANALYSIS.md`
* `TEAM_WORKFLOW.md`

---

## Status

🚧 **Under Development**

The repository currently contains the project skeleton. Individual components will be implemented incrementally.
