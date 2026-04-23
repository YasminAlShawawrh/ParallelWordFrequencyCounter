# Parallel Word Frequency Counter — C (Naive, Multiprocessing, Multithreading)

This project analyzes the performance of different **parallel computing techniques** for counting word frequencies in a large text file.  
It compares three approaches:

- Naive (Sequential)
- Multiprocessing (Multiple Processes)
- Multithreading (POSIX Threads)

The goal is to evaluate execution time, scalability, and efficiency when processing large datasets.

 Full report: :contentReference[oaicite:0]{index=0}  

---

## able of contents

- [Overview](#overview)
- [Approaches](#approaches)
- [Program structure](#program-structure)
- [Performance analysis](#performance-analysis)
- [Results](#results)
- [Limitations](#limitations)

---

## Overview

The program:
1. Reads a large text file (`large_file.txt`)
2. Extracts all words
3. Counts frequency of each word
4. Outputs the **Top 10 most frequent words**

Each approach differs in how it processes the data:
- Sequential vs Parallel
- Shared memory vs Separate memory
- Performance vs Complexity trade-offs

---

## Approaches

### Naive Approach
- Sequential execution (no parallelism)
- Reads file → sorts → counts frequencies → sorts again
- Serves as a **baseline for comparison**

Key idea:
> Simple but slow for large datasets :contentReference[oaicite:1]{index=1}  

---

### Multiprocessing Approach
- Uses multiple **child processes (fork)**
- Splits data into chunks
- Each process computes frequencies independently
- Results merged at the end

Characteristics:
- Separate memory spaces (no race conditions)
- Requires merging step
- Scales well with CPU cores

---

### Multithreading Approach
- Uses **POSIX threads (pthreads)**
- Threads share the same memory
- Each thread processes part of the dataset

Characteristics:
- Faster due to shared memory
- Risk of **race conditions** (no synchronization used) :contentReference[oaicite:2]{index=2}  

---

## Program structure
├── NaiveCode.txt           # Sequential implementation
├── MultiprocessingCode.txt # Fork-based parallel version
├── MultithreadingCode.txt  # pthread-based version
└── OS_ProjectOne.pdf       # Full report


---

## Key Components

- **read_file()** → loads words into memory  
- **qsort()** → sorts words alphabetically  
- **Frequency counting** → counts occurrences  
- **Top-K extraction** → prints top 10 words  

Parallel versions add:
- `fork()` → multiprocessing  
- `pthread_create()` → multithreading  

---

## Performance analysis

### Naive
- Fully sequential
- ~98% of execution is **serial** :contentReference[oaicite:3]{index=3}  
- Slowest approach

---

### Multiprocessing
- Best performance at **6 processes**
- Too many processes → overhead (context switching)

From report (page 16–17):
- 2 processes → slower
- 6 processes → fastest (~138 sec)
- 8 processes → slower again due to overhead :contentReference[oaicite:4]{index=4}  

---

### Multithreading
- Fastest overall (~10 sec)
- Scales with number of threads
- But introduces **race conditions**

From report (page 22):
- 8 threads → best performance
- Slight improvements beyond 4 threads :contentReference[oaicite:5]{index=5}  

---

## Results

| Approach | Execution Time |
|----------|--------------|
| Naive | ~224 sec |
| 6 Processes | ~138 sec |
| 8 Threads | ~10 sec |

Key insight:
- Multithreading is fastest  
- Multiprocessing is safest  
- Naive is simplest but slowest  

---

## Limitations

- Multithreading:
  - Race conditions (no locks used)
- Multiprocessing:
  - Overhead from process creation & merging
- Naive:
  - Not scalable for large datasets

---




