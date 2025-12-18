# Tango-Trees-Implementation

## Overview
This repository contains a C++ implementation of the Tango Tree data structure, an advanced self-adjusting binary search tree designed to support efficient dynamic search operations. Tango Trees achieve a competitive ratio of O(log log n) relative to the optimal offline BST by adapting their structure based on access patterns.

The project focuses on translating the theoretical design of Tango Trees into a working, pointer-based implementation while preserving their amortized performance guarantees.

## Key Ideas
Tango Trees are built using the following core concepts:
- Preferred Path Decomposition, where frequently accessed paths are identified dynamically
- Auxiliary Binary Search Trees to represent preferred paths efficiently
- Dynamic restructuring of the tree after each access to reflect updated preferences

These techniques allow the data structure to adapt to sequences of searches while maintaining near-optimal performance.

## Implementation Details
- Language: C++
- Pointer-based tree representation
- Implementation of:
  - Search operations across auxiliary trees
  - Tree rotations and structural updates
  - Maintenance and rebuilding of auxiliary BSTs as preferred paths change

Care is taken to ensure correctness during path splits, merges, and rotations, which are critical to the performance and correctness of Tango Trees.

## Analysis
The project applies amortized and competitive analysis to evaluate access costs relative to the optimal BST. The implementation follows the standard Tango Tree framework and demonstrates how theoretical guarantees can be preserved in practice.

## Learning Outcomes
- Practical understanding of advanced self-adjusting data structures
- Experience with amortized and competitive analysis
- Improved proficiency in low-level C++ pointer manipulation
