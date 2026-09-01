# ⚡ CmpE 322 - Multithreaded Parallel Hash Table in C

![C](https://img.shields.io/badge/Language-C-A8B9CC?style=for-the-badge&logo=c&logoColor=white)
![POSIX Threads](https://img.shields.io/badge/Concurrency-Pthreads-blue?style=for-the-badge)
![Operating Systems](https://img.shields.io/badge/Course-CmpE_322_Operating_Systems-darkgreen?style=for-the-badge)
![Concept](https://img.shields.io/badge/Concept-Fine--Grained_Locking-orange?style=for-the-badge)

---

## 📌 Executive Summary

This repository contains a high-performance **Multithreaded Parallel Hash Table** implemented in C for **CmpE 322 (Operating Systems)** at **Boğaziçi University**. 

The project focuses on multi-threaded concurrent data structures, fine-grained mutex locking mechanisms, deadlock/livelock prevention strategies, and performance benchmarking (Speedup ratio & Amdahl's Law) using POSIX Threads (`pthread`).

---

## 🌟 Key Technical Concepts & Locking Strategies

### 1. Fine-Grained Mutex Locking
Instead of using a coarse-grained global lock that serializes all table accesses, this implementation allocates an array of `pthread_mutex_t` locks—one unique mutex lock for every slot in the hash table (`lock_list[n]`). This dramatically reduces lock contention and allows independent threads to insert items concurrently into different slots.

### 2. Parallel Linear Probing (`parallel_h_1`)
- Input entries are partitioned evenly across `t` worker threads (`m/t` items per thread).
- **Double-Checked Locking Pattern:** Before acquiring a slot's mutex, a thread checks if the slot is empty. If empty, it attempts `pthread_mutex_trylock(&lock_list[index])`. Upon acquiring the lock, it re-verifies that the slot is still `NULL` before inserting the item and timestamping it with `clock_gettime(CLOCK_MONOTONIC)`.
- Minimizes thread blocking time while maintaining strict thread safety.

### 3. Double Hashing & Deadlock Prevention (`parallel_h_2`)
- In multi-slot reservation probing (Double Hashing), threads need to verify a group of $n/k$ slots.
- **Rollback & Retry (Deadlock Avoidance):** Acquiring multiple locks sequentially can cause circular wait deadlocks. To prevent this, threads attempt non-blocking lock acquisition using `pthread_mutex_trylock`. If any lock in the set cannot be acquired immediately, the thread **releases all locks it currently holds** and restarts the probing sequence. This guarantees lock-freedom and complete deadlock avoidance.

### 4. Speedup Benchmarking & Performance Analysis (`speedup_comparison_h_1`)
- Evaluates execution time using monotonic high-resolution timers (`clock_gettime(CLOCK_MONOTONIC)`).
- Calculates empirical Speedup:
  $$\text{Speedup} = \frac{T_{\text{sequential}}}{T_{\text{parallel}}}$$
- Demonstrates how fine-grained locking scales performance with multi-core CPUs according to Amdahl's Law.

---

## 📂 Repository Structure

```
CmpE322-Multithreaded-Parallel-Hash-Table/
├── hash_parallelization.h     # Data structures, thread parameters, & function prototypes
├── hash_parallelization.c     # Core implementation (sequential, parallel_h1, parallel_h2, speedup)
├── test_main.c                # Test harness & benchmark runner
├── Makefile                   # Build automation script (gcc -Wall -O2 -pthread)
├── report.txt                 # Detailed engineering report
├── cmpe322_project2.pdf       # Official course project specification
├── PROJECT2 (1).pdf           # Detailed project requirements document
├── .gitattributes             # Ensures Linux LF line endings across C sources & Makefile
└── .gitignore                 # Excludes compiled binaries (*.o, executables)
```

---

## 🚀 How to Build & Run

### Prerequisites
- GCC Compiler with POSIX Threads support (`pthread`).
- Linux / macOS / WSL environment.

### 1. Build the Project
Compile using the included `Makefile`:
```bash
make
```

### 2. Run the Benchmark & Tests
Execute the compiled binary:
```bash
./test_main
```

### 3. Clean Build Artifacts
```bash
make clean
```

---

## 👨‍💻 Author

**Ahmet Meriç Kızıltaş**  
*Department of Computer Engineering, Boğaziçi University*  
[Student ID: 2022400225]
