#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <optional>
#include <unordered_map>

#include "hash.hpp"
#include "memhash.hpp"
#include "vertex.hpp"

namespace {

    using vertex_digest_pair = std::tuple<vertex const *, hash::digest>;

    // Iterator is an STL iterator type which will produce an instance of type vertex_digest_pair.
    template <typename Iterator>
    void dump (std::ostream & os, Iterator first, Iterator last) {
        char const * separator = "  ";
        std::for_each (first, last, [&] (auto const & t) {
            os << separator << std::get<vertex const *> (t)->name () << ':' << std::hex
               << std::get<hash::digest> (t);
            separator = ", ";
        });
        os << '\n';
    }

    /// \tparam Iterator An iterator type which will produce an instance of type vertex_digest_pair.
    template <typename Iterator>
    std::vector<vertex_digest_pair> hash_vertices (Iterator first, Iterator last) {
        std::vector<vertex_digest_pair> digests;
        memoized_hashes table;
        std::for_each (first, last, [&digests, &table] (vertex const & v) {
            digests.emplace_back (&v, vertex_hash (&v, &table));
        });

        std::sort (std::begin (digests), std::end (digests),
                   [] (vertex_digest_pair const & x, vertex_digest_pair const & y) {
                       return std::get<0> (x)->name () < std::get<0> (y)->name ();
                   });
        dump (std::cout, std::begin (digests), std::end (digests));
        return digests;
    }

    /// digraph G {
    ///    c -> a;
    ///    c -> b;
    /// }
    void test1 () {
        enum { va, vb, vc };
        std::array<vertex, 3> graph{{vertex{"a"}, vertex{"b"}, vertex{"c"}}};
        graph[vc].add_adjacent ({&graph[va], &graph[vb]});

        auto const h1 = hash_vertices (std::begin (graph), std::end (graph));
        auto const h2 = hash_vertices (std::rbegin (graph), std::rend (graph));
        assert (std::equal (std::begin (h1), std::end (h1), std::begin (h2), std::end (h2)));
    }

    /// digraph G {
    ///    a -> b;
    ///    b -> a;
    ///    c -> a;
    /// }
    ///
    /// The result hashes shoud be:
    /// | Name | |
    /// | "a" | Va/Vb/R0<br>
    ///         Vertex "a" -> vertex "b" -> loop back to the initial (zeroth) vertex. The slash in
    ///         this notation can be thought of a directed edge between the two adjacent vertices.
    /// | "b" | Vb/Va/R0 |
    /// | "c" | Vc/Va/Vb/R1 |
    void test_looped () {
        enum { va, vb, vc };
        std::array<vertex, 3> graph{{vertex{"a"}, vertex{"b"}, vertex{"c"}}};
        graph[va].add_adjacent ({&graph[vb]});
        graph[vb].add_adjacent ({&graph[va]});
        graph[vc].add_adjacent ({&graph[va]});

        auto const h1 = hash_vertices (std::begin (graph), std::end (graph));
        auto const h2 = hash_vertices (std::rbegin (graph), std::rend (graph));
        assert (std::equal (std::begin (h1), std::end (h1), std::begin (h2), std::end (h2)));
    }

} // end anonymous namespace

int main () {
    std::cout << "simple test:\n";
    test1 ();
    std::cout << "Bytes hashed for far: " << std::dec << hash::total () << '\n';
    std::cout << "test with loop:\n";
    test_looped ();
    std::cout << "Total bytes hashed: " << std::dec << hash::total () << '\n';
}
