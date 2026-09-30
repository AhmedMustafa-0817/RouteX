# Contributing to ROUTEX

## 1. Team Members

* Ahmed Mustafa
* Jawwad
* Hania

---

# 2. Branch Structure

ROUTEX uses:

```text
main
develop
feature/*
fix/*
docs/*
```

### main

Stable, tested, submission-ready code.

### develop

Integrated team development branch.

### feature/*

Used for new functionality.

Examples:

```text
feature/order-queue
feature/hash-table
feature/graph
feature/dijkstra
feature/blockchain
```

### fix/*

Used for bug fixes.

Examples:

```text
fix/queue-empty-case
fix/dijkstra-no-path
```

### docs/*

Used for documentation-only changes.

---

# 3. Standard Workflow

Start from the latest `develop`:

```bash
git switch develop
git pull origin develop
```

Create a feature branch:

```bash
git switch -c feature/your-feature
```

Work and test locally.

Check your changes:

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

Create a Pull Request:

```text
feature/your-feature → develop
```

Another team member reviews the Pull Request.

After approval and successful testing, merge it into `develop`.

---

# 4. Before Starting New Work

Always synchronize your local `develop`:

```bash
git switch develop
git pull origin develop
```

Then create the new feature branch.

This prevents you from developing on an outdated version of the project.

---

# 5. Pull Request Requirements

A Pull Request should contain:

* Clear title.
* Description of changes.
* Files changed.
* Testing performed.
* Known issues, if any.
* Handoff information for the next developer.

Example:

```text
Title:
Implement FIFO shipment queue

Changes:
- Added Order class.
- Added Queue class.
- Implemented enqueue().
- Implemented dequeue().
- Implemented front().
- Added queue tests.

Testing:
- Empty queue
- Single order
- Multiple orders
- FIFO processing
```

---

# 6. Coding Rules

1. Do not directly develop on `main`.
2. Avoid directly changing `develop`.
3. Never commit build files.
4. Never commit passwords or authentication tokens.
5. Keep header and source files separated.
6. Use meaningful variable and function names.
7. Keep functions reasonably small.
8. Test your changes before opening a Pull Request.
9. Do not change unrelated files.
10. Keep `main.cpp` small.
11. Avoid unnecessary external libraries.
12. Do not add features outside the agreed project scope without team agreement.

---

# 7. Commit Message Style

Use descriptive messages:

```text
Implement FIFO shipment queue
Implement hash table collision handling
Add weighted graph
Implement min heap
Implement Dijkstra shortest travel time
Add blockchain block structure
Implement SHA-256 hashing
Add blockchain verification
```

Avoid:

```text
Update
Done
Final
Changes
test
```

---

# 8. Build Before Pull Request

Mac:

```bash
cmake -B build
cmake --build build
```

Windows:

```powershell
cmake -B build
cmake --build build
```

The project should build successfully before requesting review.

---

# 9. Merge Conflicts

If a merge conflict occurs:

1. Do not delete someone else's work blindly.
2. Read both versions.
3. Decide which code should remain.
4. Remove Git conflict markers.
5. Build and test again.
6. Commit the resolution.

Useful commands:

```bash
git status
git add .
git commit -m "Resolve merge conflict"
git push
```

---

# 10. Daily Handoff

At the end of each development day, record:

```text
DAY:
OWNER:

COMPLETED:

FILES CHANGED:

TESTS PERFORMED:

KNOWN ISSUES:

NEXT DEVELOPER:

NEXT TASK:
```

The purpose is to make the daily rotating workflow smooth.

---

# 11. Scope Protection

ROUTEX is a DSA project.

Do not add major new technologies such as:

* Machine Learning
* GPS
* Cryptocurrency
* Smart Contracts
* AI
* Payment Systems
* Mobile Applications

unless the entire team and instructor approve a scope change.

The core project must remain stable.
