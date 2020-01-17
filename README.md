# Directed Graph Hash

[![License](https://img.shields.io/github/license/paulhuggett/digraph-hash)](https://img.shields.io/github/license/paulhuggett/digraph-hash)
[![Build Status](https://travis-ci.com/paulhuggett/digraph-hash.svg?branch=master)](https://travis-ci.com/paulhuggett/digraph-hash)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=paulhuggett_digraph-hash&metric=alert_status)](https://sonarcloud.io/dashboard?id=paulhuggett_digraph-hash)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/a19f09751c9740eb8422617877da1470)](https://www.codacy.com/manual/paulhuggett/digraph-hash?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=paulhuggett/digraph-hash&amp;utm_campaign=Badge_Grade)
[![Total alerts](https://img.shields.io/lgtm/alerts/g/paulhuggett/digraph-hash.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/paulhuggett/digraph-hash/alerts/)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/paulhuggett/digraph-hash.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/paulhuggett/digraph-hash/context:cpp)

This program demonstrates a method of generating identifying hashes for each vertex in a, potentially cyclic, directed graph. Where possible, intermediate results are memoized. This means that we an improve performance in cases where a highly connected but acyclic path is encountered. 

## Table of Contents

*   [Directed Graph Hash](#directed-graph-hash)
    *   [Building the code](#building-the-code)
        *   [Dependencies](#dependencies)
        *   [Clone, Configure and Build](#clone-configure-and-build)
    *   [Notation](#notation)
    *   [Examples](#examples)
        *   [A Simple Example](#a-simple-example)
        *   [A Looping Example](#a-looping-example)
        *   [A Self\-Loop](#a-self-loop)
        *   [Hybrid Example](#hybrid-example)

## Building the code

### Dependencies

*   [CMake 3.10](https://cmake.org/download/) or later
*   A C++17 compiler. The code is tested with Xcode 9.3, GCC 9.2, and Microsoft Visual Studio 2017.

### Clone, Configure and Build

We need to first clone the source code and its git submodules from github. Next, we create a directory for the build. Running cmake with an optional argument to select a specific build system (“generator”) creates the build. This allows us to use the build utility of our choice (Make, Ninja, Xcode, Visual Studio) to build the code.

~~~bash
git clone --recursive https://github.com/paulhuggett/digraph-hash.git
cd digraph-hash
mkdir build
cd build
cmake ..
cd ..
cmake --build build
~~~

### Notation

The table below describes the notation used by the string-hash output and for the result digests shown in the examples below.

| Name    | Description |
| ------- | ----------- |
| *x*/*y* | A directed edge from vertex *x* to vertex *y* |
| E       | Indicates that all of the edges from a vertex have been encoded. This is necessary to enable the hash to distinguish: `a → b → c;` (“Va/Vb/VcEE”) and `a → b; a → c;` (“Va/VbE/VcEE”) |
| R*n*    | A “back reference” to the *n*th preceeding vertex record in the encoding. For example, in an identity-relationship *n* is zero (`a → a;`) so it would be always encoded as “Va/R0E” (enabling the result to be memoized); a loop between two adjacent vertices (`a -> b -> a;`) becomes “Va/Vb/R1EE” |
| V*x*    | Entry for vertex *x* |

## Examples

### A Simple Example

Looking at the graph below:

[![Simple example](https://sketchviz.com/@paulhuggett/4219c7ba02ac32a9a14c9566bb526ffa/e057ec6efb6522c45a1c8d404618406f7dac2d62.sketchy.png)](https://sketchviz.com/@paulhuggett/4219c7ba02ac32a9a14c9566bb526ffa)

If vertex “a” is visited first, we generate its hash and memoize it. Likewise for vertex “b”. When generating the hash for “c”, we can reuse the cached results for the two connected vertices. Conversely, if we were to visit the vertices in the opposite order (traversing the paths from “c” first), we would have already cached the results for both “a” and “b” when they are themselves visited.

| Vertex | Encoding      | Cached? |
| ------ | ------------- | ------- |
| a      | VaE           | Yes     |
| b      | VbE           | Yes     |
| c      | Vc/VaE/VbEE   | Yes     |

### A Looping Example

[![Looping example](https://sketchviz.com/@paulhuggett/4219c7ba02ac32a9a14c9566bb526ffa/4ceba724fba0e4d34457a3bfd6b92b7b5bbf2fe6.sketchy.png)](https://sketchviz.com/@paulhuggett/4219c7ba02ac32a9a14c9566bb526ffa)

Here we have a cycle between vertices “a” and “b”. In order to be able to record the looping edge we need to encode a “back-reference” to a previously visited vertex. This is done using the index of that vertex which can change depending where the traversal begins. This means that we cannot memoize the results for any of the vertices in this example.

| Vertex | Encoding         | Cached? |
| ------ | ---------------- | ------- |
| a      | Va/Vb/R1EE       | No      |
| b      | Vb/Va/R1EE       | No      |
| c      | Vc/Va/Vb/R1EEE   | No      |

### A Self-Loop

[![Self-Loop](https://sketchviz.com/@paulhuggett/9729072eeb157bba5a3fd59bdfcaaa65/9c514e2eac2a20ac26eaca13cd9637035fc13513.sketchy.png)](https:://sketchviz.com/@paulhuggett/9729072eeb157bba5a3fd59bdfcaaa65)

Here a vertex contains an edge back to the same vertex. We can treat this case specially and cache the result because the loop has just a single possible entry-point.

| Vertex | Encoding      | Cached? |
| ------ | ------------- | ------- |
| a      | Va/R0E        | Yes     |

### Hybrid Example

[![Hybrid Example](https://sketchviz.com/@paulhuggett/1b8fba83ebff1ebe66aa96a9fa9d7c2e/7c0fb7ef6465c31f858940b444b2cf6b2f66d748.sketchy.png)](https://sketchviz.com/@paulhuggett/1b8fba83ebff1ebe66aa96a9fa9d7c2e)

This time we combine both examples: from vertex “a” we can reach a cluster of looping nodes ( “b” → “c” → “b” ) as well as an acyclic group ( “a” → “d” and so on).

| Vertex | Encoding                   | Cached? |
| ------ | -------------------------- | ------- |
| a      | Va/Vb/Vc/R1EE/Vd/VeE/VfEEE | No      |
| b      | Vb/Vc/R0EE                 | No      |
| c      | Vc/Vb/R0EE                 | No      |
| d      | Vd/VeE/VfEE                | Yes     |
| e      | VeE                        | Yes     |
| f      | VfE                        | Yes     |
