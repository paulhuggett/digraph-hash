# Directed Graph Hash

[![License](https://img.shields.io/github/license/paulhuggett/digraph-hash)](https://img.shields.io/github/license/paulhuggett/digraph-hash)
[![Build Status](https://travis-ci.com/paulhuggett/digraph-hash.svg?branch=master)](https://travis-ci.com/paulhuggett/digraph-hash)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=paulhuggett_digraph-hash&metric=alert_status)](https://sonarcloud.io/dashboard?id=paulhuggett_digraph-hash)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/a19f09751c9740eb8422617877da1470)](https://www.codacy.com/manual/paulhuggett/digraph-hash?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=paulhuggett/digraph-hash&amp;utm_campaign=Badge_Grade)
[![Total alerts](https://img.shields.io/lgtm/alerts/g/paulhuggett/digraph-hash.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/paulhuggett/digraph-hash/alerts/)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/paulhuggett/digraph-hash.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/paulhuggett/digraph-hash/context:cpp)
[![BCH compliance](https://bettercodehub.com/edge/badge/paulhuggett/digraph-hash?branch=master)](https://bettercodehub.com/)

This program demonstrates a method of generating identifying hashes for each vertex in a potentially cyclic directed graph. Where possible, intermediate results are memoized. This means that we can improve performance in cases where a highly connected but acyclic path is encountered.

To read about the algorithm and see some examples of it at work, please visit the [digraph-hash home page](https://paulhuggett.github.io/digraph-hash).
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
