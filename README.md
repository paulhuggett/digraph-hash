# Directed Graph Hash

[![License](https://img.shields.io/github/license/paulhuggett/digraph-hash)](https://img.shields.io/github/license/paulhuggett/digraph-hash)
[![Build Status](https://travis-ci.com/paulhuggett/digraph-hash.svg?branch=master)](https://travis-ci.com/paulhuggett/digraph-hash)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=paulhuggett_digraph-hash&metric=alert_status)](https://sonarcloud.io/dashboard?id=paulhuggett_digraph-hash)
[![Total alerts](https://img.shields.io/lgtm/alerts/g/paulhuggett/digraph-hash.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/paulhuggett/digraph-hash/alerts/)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/paulhuggett/digraph-hash.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/paulhuggett/digraph-hash/context:cpp)

This program demonstrates a method of generating identifying hashes for each vertex in a, potentially cyclic, directed graph. Where possible, intermediate results are memoized. This means that we an improve performance in cases where a highly connected but acyclic path is encountered. 


## Building the code

### Dependencies

- [CMake 3.10](https://cmake.org/download/) or later
- A C++17 compiler. The code is tested with Xcode 9.3, GCC 9.2, and Microsoft Visual Studio 2017.

### Clone, Configure and Build

We need to first clone the source code and its git submodules from github. Next, we create a directory for the build. Running cmake with an optional argument to select a specific build system (“generator”) creates the build. This allows us to use the build utility of our choice (Make, Ninja, Xcode, Visual Studio) to build the code.

~~~bash
$ git clone --recursive https://github.com/paulhuggett/digraph-hash.git
$ cd digraph-hash
$ mkdir build
$ cd build
$ cmake .. <optional generator>
$ cd ..
$ cmake --build build
~~~

## Examples

### A Simple Example

Looking at the graph below:

[![Simple example](https://sketchviz.com/@paulhuggett/4219c7ba02ac32a9a14c9566bb526ffa/e057ec6efb6522c45a1c8d404618406f7dac2d62.sketchy.png)](https://sketchviz.com/@paulhuggett/4219c7ba02ac32a9a14c9566bb526ffa)

If vertex “a” is visited first, we generate its hash and memoize it. Likewise for vertex “b”. When generating the hash for “c”, we can reuse the cached results for the two connected vertices.

| Vertex | Encoding   | Cached? |
| ------ | ---------- | ------- |
| “a”    | `Va`       | Yes     |
| “b”    | `Vb`       | Yes     |
| “c”    | `Vc/Va/Vb` | Yes     |

(In the notation used here _Vx_ refers to the encoded hash of vertex _x_. A slash can be thought of as an edge between the preceeding and succeeding vertices.)

### A Looping Example

[![Looping example](https://sketchviz.com/@paulhuggett/4219c7ba02ac32a9a14c9566bb526ffa/4ceba724fba0e4d34457a3bfd6b92b7b5bbf2fe6.sketchy.png)](https://sketchviz.com/@paulhuggett/4219c7ba02ac32a9a14c9566bb526ffa)

Here we have a cycle between vertices “a” and “b”. In order to be able to record the looping edge we need to encode a "back-reference" to a previously visited vertex. This is done using the index of that vertex which can change depending where the traversal begins. This means that we cannot memoize the results for any of the vertices in this example.

| Vertex | Encoding      | Cached? |
| ------ | ------------- | ------- |
| “a”    | `Va/Vb/R0`    | No      |
| “b”    | `Vb/Va/R0`    | No      |
| “c”    | `Vc/Va/Vb/R1` | No      |

(Here the notation uses _Rn_ to indicate a back-reference to the vertex at zero-based index _n_.)

### Hybrid Example

[![Hybrid Example](https://sketchviz.com/@paulhuggett/1b8fba83ebff1ebe66aa96a9fa9d7c2e/7c0fb7ef6465c31f858940b444b2cf6b2f66d748.sketchy.png)](https://sketchviz.com/@paulhuggett/1b8fba83ebff1ebe66aa96a9fa9d7c2e)

This time we combine both examples: from vertex “a” we can reach a cluster of looping nodes ( “b” → “c” → “b” ) as well as an acyclic group ( “a” → “d” and so on).

| Vertex | Encoding               | Cached? |
| ------ | ---------------------- | ------- |
| “a”    | `Va/Vb/Vc/R1/Vd/Ve/Vf` | No      |
| “b”    | `Vb/Vc/R0`             | No      |
| “c”    | `Vc/Vb/R0`             | No      |
| “d”    | `Vd/Ve/Vf`             | Yes     |
| “e”    | `Ve`                   | Yes     |
| “f”    | `Vf`                   | Yes     |