# Set-Associative Caches: A Performance Simulator

**Course:** CSCE 2303 – Computer Organization and Assembly Language Programming  
**Authors:** Ahmed Soliman, Saifeldin Abdulrahman  
**Instructor:** Dr. Mohamed Shalan  
**Term:** Spring 2025  

[GitHub Repository](https://github.com/saifsabry21/AssemblyProject2.git)

---

## 📘 Abstract

This simulator models a 64 KiB set-associative cache and evaluates its performance under various configurations and memory access patterns. It features:

- A configurable cache line size (16, 32, 64, 128 bytes).
- Varying associativity levels (1, 2, 4, 8, 16 ways).
- Six memory access generators (from full DRAM sequential to randomized access).
- One million memory references per experiment.
- Comparison between Set-Associative (SA) designs.
- Visualization of hit/miss ratios via plots.
- Analysis of differnet configurations for different memory access patterns
- Five test cases that verify the caches functionality

---

## 🧠 Introduction

Caches bridge the speed gap between CPU registers and main memory. This simulator evaluates how cache performance is affected by:

- **Cache line size**: affects spatial locality.
- **Associativity**: reduces conflict misses.

Two core questions are explored:

1. How does changing the **line size** affect hit/miss ratios in a 4-set SA cache?
2. How does varying **associativity** affect performance when using a 64 B line size?

---

## ⚙️ Methodology

### 1. Design

- **Cache Model**: 64 KiB cache in front of a 64 MiB DRAM.
- **SA Implementation**:
  ```cpp
  struct CacheLineFA {
      unsigned int tag = 0;
      bool valid = false;
      unsigned int leastrecentuseCounter = 0;
  };

## 👥 Contributions
**Saifeldin Abdulrahman**

- Designed and implemented Experiment 2.

- Completed five functional test cases to verify cache simulator correctness.

- Collected and analyzed data for Experiment 2.

- Wrote the Abstract, Introduction, and Methodology sections of the report.

- Participated in writing the Discussion for Experiment 2.

- Collaborated on developing the cache simulator code.

**Ahmed Soliman
**
- Designed and implemented Experiment 1.

- Collected and analyzed data for Experiment 1.

- Created graphical representations for both experiments.

- Wrote the Discussion for Experiment 1.

- Authored the Conclusion section of the report.

- Collaborated on developing the cache simulator code.


