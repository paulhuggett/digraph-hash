# Directed Graph Hash

[![License](https://img.shields.io/github/license/paulhuggett/digraph-hash)](https://img.shields.io/github/license/paulhuggett/digraph-hash)
[![Build Status](https://travis-ci.com/paulhuggett/digraph-hash.svg?branch=master)](https://travis-ci.com/paulhuggett/digraph-hash)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=paulhuggett_digraph-hash&metric=alert_status)](https://sonarcloud.io/dashboard?id=paulhuggett_digraph-hash)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/a19f09751c9740eb8422617877da1470)](https://www.codacy.com/manual/paulhuggett/digraph-hash?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=paulhuggett/digraph-hash&amp;utm_campaign=Badge_Grade)
[![Total alerts](https://img.shields.io/lgtm/alerts/g/paulhuggett/digraph-hash.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/paulhuggett/digraph-hash/alerts/)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/paulhuggett/digraph-hash.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/paulhuggett/digraph-hash/context:cpp)
[![BCH compliance](https://bettercodehub.com/edge/badge/paulhuggett/digraph-hash?branch=master)](https://bettercodehub.com/)

This program demonstrates a method of generating identifying hashes for each vertex in a, potentially cyclic, directed graph. Where possible, intermediate results are memoized. This means that we can improve performance in cases where a highly connected but acyclic path is encountered.

## Table of Contents

*   [Building the code](#building-the-code)
    *   [Dependencies](#dependencies)
    *   [Clone, Configure and Build](#clone-configure-and-build)
*   [The Algorithm](#the-algorithm)
    *   [Origins](#origins)
*   [Examples](#examples)
    *   [Notation](#notation)
    *   [A Simple Example](#a-simple-example)
    *   [A Looping Example](#a-looping-example)
    *   [A Self\-Loop](#a-self-loop)
    *   [Hybrid Example](#hybrid-example)

## Building the code

### Dependencies

*   [CMake 3.10](https://cmake.org/download/) or later
*   A C++17 compiler. The code is tested with Xcode 9.3, GCC 9.2, and Microsoft Visual Studio 2017.

### Clone, Configure and Build

We need to first clone the source code and its git submodules from github. Next, we create a directory for the build. Running cmake with an optional argument to select a specific build system (“generator”) creates the build. This allows us to use the build system of our choice (Make, Ninja, Xcode, Visual Studio, and so on) to build the code.

~~~bash
git clone --recursive https://github.com/paulhuggett/digraph-hash.git
cd digraph-hash
mkdir build
cd build
cmake ..
cd ..
cmake --build build
~~~

## The Algorithm

~~~
function vertex-hash (v is a vertex,
                      table is an associative array (vertex→digest),
                      visited is an associative array (vertex→integer))
    num-visited ← size of associative array "visited"
    if v in table then
        return (num-visited, table[v])
    h ← new cryptographic hash
    if v in visited then
        h ⨁ back_reference(num-visited - visited[v] - 1)
        return (visited[v], hash-finalize(h))

    visited[v] ← num-visited
    h ⨁ v
    loop-point ← MAXIMUM
    for each out-edge e of vertex v do
        h ⨁ e
        out ← target vertex of e
        (lp, digest) ← vertex-hash (out, table, visited)
        if out ≠ v then
            loop-point ← min(loop-point, lp)
        h ⨁ digest
    h ⨁ end
    digest ← hash-finalize(h)
    if loop-point > num-visited then
        table[v] ← digest
    del v from visited
    return (loop-point, digest)
~~~

The goal of this algorithm is to produce an identifying hash for each vertex in a directed, and potentially cyclic, graph. The hash of each vertex is produced from the attributes of the vertex itself, its out-going edges and its connected vertices. Highly connected graphs can result in having to visit the same vertices repeatedly so, to provide the best performance, whenever possible we would like to memoize the hash of vertices that we visit.

If the input graph is known to be directed-acyclic, then the algorithm is straightfoward since the hash of each vertex can be safely memoized once it has been computed. If an edge from another vertex connects to it then the hash is returned immediately. The possibility of loops in the graph mean that a more sophisticated approach is needed.

Where several vertices may form a loop (directed-cyclic), the sequence in which the linked vertices are visited will depend on the point of entry into the loop. For example:

[![Loop with two entry points](https://sketchviz.com/@paulhuggett/4219c7ba02ac32a9a14c9566bb526ffa/49d199d7c5d87a9f63553ef27b5c938a1be639ed.png)](//sketchviz.com/@paulhuggett/4219c7ba02ac32a9a14c9566bb526ffa)

Traversing the graph starting with vertex “a” will visit the graph in the order `a → c → d → c` before stopping (due to  having arrived back at a previously encountered vertex). On the other hand, a traversal starting with “b” will follow the order `b → d → c → d` before stopping. This has the result that we cannot memoize the hash for either vertex “c” or vertex “d”. Vertices “a” and “b” lie outside of the `c → d → c` loop, so it is possible for us to safely memoize these hashes. Note that, as a special case, an out-going edge of a vertex which loops back to itself (a “self loop”) can be safely memoized as there is only a single point-of-entry to the loop.

### Origins

The earliest version of this algorithm was based on the method used to eliminate duplicate types from DWARF debugging information (see [DWARF Debugging Information Format Version 4]((http://dwarfstd.org/doc/DWARF4.pdf)), Appendix E.2). In this scheme, each type is emitted to a separate `.debug_types` object-file section which has an associated “key” calculated by hashing the contents of the type and of all types that are reachable from it.

## Examples

### Notation

The table below describes the notation used by the string-hash output and for the result digests shown in the examples below.

| Name    | Description |
| ------- | ----------- |
| *x*/*y* | A directed edge from vertex *x* to vertex *y* |
| E       | Indicates that all of the edges from a vertex have been encoded. This is necessary to enable the hash to distinguish: `a → b → c;` (“Va/Vb/VcEE”) and `a → b; a → c;` (“Va/VbE/VcEE”) |
| R*n*    | A “back reference” to the *n*th preceding vertex record in the encoding. *n* is calculated by counting backwards through the list of previously visited vertices. In an identity-relationship, *n* is zero (`a → a;`) so it would be always encoded as “Va/R0E”; a loop between two adjacent vertices (`a → b → a;`) becomes “Va/Vb/R1EE” |
| V*x*    | Entry for vertex *x* |

### A Simple Example

Looking at the graph below:

[![Simple example](https://sketchviz.com/@paulhuggett/4219c7ba02ac32a9a14c9566bb526ffa/e057ec6efb6522c45a1c8d404618406f7dac2d62.sketchy.png)](https://sketchviz.com/@paulhuggett/4219c7ba02ac32a9a14c9566bb526ffa)

If vertex “a” is visited first, we generate its hash and memoize it; likewise for vertex “b”. When generating the hash for “c”, we can reuse the cached results for the two connected vertices. Conversely, if we were to visit the vertices in the opposite order (traversing the paths from “c” first), we would have already cached the results for both “a” and “b” when they are themselves visited.

| Vertex | Encoding      | Cached? |
| ------ | ------------- | ------- |
| a      | VaE           | Yes     |
| b      | VbE           | Yes     |
| c      | Vc/VaE/VbEE   | Yes     |

### A Looping Example

[![Looping example](https://sketchviz.com/@paulhuggett/4219c7ba02ac32a9a14c9566bb526ffa/4ceba724fba0e4d34457a3bfd6b92b7b5bbf2fe6.sketchy.png)](https://sketchviz.com/@paulhuggett/4219c7ba02ac32a9a14c9566bb526ffa)

Here we have a cycle between vertices “a” and “b”. In order to be able to record the looping edge we need to encode a “back-reference” to a previously visited vertex. This is done the index of that vertex relative to the current position: R0 encodes an edge that points to itself, R1 references to the preceeding vertex in the encoding, and so on. This means that we can memoize the results for vertices have edges to members of a loop but that aren’t included within the loop.

| Vertex | Encoding         | Cached? |
| ------ | ---------------- | ------- |
| a      | Va/Vb/R1EE       | No      |
| b      | Vb/Va/R1EE       | No      |
| c      | Vc/Va/Vb/R1EEE   | Yes     |

### A Self-Loop

[![Self-Loop](https://sketchviz.com/@paulhuggett/45b6427f8db91b04a997420124ded8b4/5aeacb40050b50397222e6f97789b34029f59e29.sketchy.png)](https:://sketchviz.com/@paulhuggett/45b6427f8db91b04a997420124ded8b4)

Here a vertex contains an edge back to the same vertex. We can treat this case specially and cache the result because the loop has just a single possible entry-point.

| Vertex | Encoding      | Cached? |
| ------ | ------------- | ------- |
| a      | Va/Vb/R0E     | Yes     |
| b      | Vb/R0E        | Yes     |

### Hybrid Example

[![Hybrid Example](https://sketchviz.com/@paulhuggett/1b8fba83ebff1ebe66aa96a9fa9d7c2e/7c0fb7ef6465c31f858940b444b2cf6b2f66d748.sketchy.png)](https://sketchviz.com/@paulhuggett/1b8fba83ebff1ebe66aa96a9fa9d7c2e)

This time we combine both examples: from vertex “a” we can reach a cluster of looping nodes ( “b” → “c” → “b” ) as well as an acyclic group ( “a” → “d” and so on).

| Vertex | Encoding                   | Cached? |
| ------ | -------------------------- | ------- |
| a      | Va/Vb/Vc/R1EE/Vd/VeE/VfEEE | Yes     |
| b      | Vb/Vc/R1EE                 | No      |
| c      | Vc/Vb/R1EE                 | No      |
| d      | Vd/VeE/VfEE                | Yes     |
| e      | VeE                        | Yes     |
| f      | VfE                        | Yes     |
