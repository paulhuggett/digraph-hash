# Directed Graph Hash

[![Total alerts](https://img.shields.io/lgtm/alerts/g/paulhuggett/digraph-hash.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/paulhuggett/digraph-hash/alerts/)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/paulhuggett/digraph-hash.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/paulhuggett/digraph-hash/context:cpp)

This program demonstrates a method of generating identifying hashes for each vertex in a, potentially cyclic, directed graph.

Where possible, the results are memoized. This means that we an improve performance in cases where a highly connected but acyclic path is encountered. 

## A simple example

Looking at the graph below:

<div><a href='//sketchviz.com/@paulhuggett/4219c7ba02ac32a9a14c9566bb526ffa'><img src='https://sketchviz.com/@paulhuggett/4219c7ba02ac32a9a14c9566bb526ffa/e057ec6efb6522c45a1c8d404618406f7dac2d62.sketchy.png' style='max-width: 100%;'></a></div>

If vertex “a” is visited first, we generate its hash and memoize it. Likewise for vertex “b”. When generating the hash for “c”, we can reuse the cached results for the two connected vertices.

| Vertex | Encoding |
| ------ | -------- |
| “a”    | Va       |
| “b”    | Vb       |
| “c”    | Vc/Va/Vb |

(In the notation used here _Vx_ refers to the encoded hash of vertex _x_. A slash can be thought of as an edge between the preceeding and succeeding vertices.)

## A looping example

<div><a href='//sketchviz.com/@paulhuggett/4219c7ba02ac32a9a14c9566bb526ffa'><img src='https://sketchviz.com/@paulhuggett/4219c7ba02ac32a9a14c9566bb526ffa/4ceba724fba0e4d34457a3bfd6b92b7b5bbf2fe6.sketchy.png' style='max-width: 100%;'></a></div>

Here we have a cycle between vertices “a” and “b”. In order to be able to record the looping edge we need to encode a "back-reference" to a previously visited vertex. This is done using the index of that vertex which can change depending where the traversal begins. This means that we cannot memoize the results for any of the vertices in this example.

| Vertex | Encoding    |
| ------ | ----------- |
| “a”    | Va/Vb/R0    |
| “b”    | Vb/Va/R0    |
| “c”    | Vc/Va/Vb/R1 |

(Here the nation uses _Rn_ to indicate a back-reference to the vertex at zero-based index _n_.)
