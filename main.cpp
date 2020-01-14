#include <algorithm>
#include <cassert>
#include <iostream>
#include <list>

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
    std::tuple<memoized_hashes, std::vector<vertex_digest_pair>> hash_vertices (Iterator first,
                                                                                Iterator last) {
        std::vector<vertex_digest_pair> digests;

        memoized_hashes table;
        std::for_each (first, last, [&digests, &table] (vertex const & v) {
            digests.emplace_back (&v, vertex_hash (&v, &table));
        });

        // Sort the result by vertex name. This means that the iteration order won't have any
        // effect on the output.
        std::sort (std::begin (digests), std::end (digests),
                   [] (vertex_digest_pair const & x, vertex_digest_pair const & y) {
                       return std::get<0> (x)->name () < std::get<0> (y)->name ();
                   });
        dump (std::cout, std::begin (digests), std::end (digests));
        return {std::move (table), std::move (digests)};
    }

    /// digraph G {
    ///    c -> a;
    ///    c -> b;
    /// }
    void test1 () {
        std::list<vertex> graph;
        auto & va = graph.emplace_back ("a");
        auto & vb = graph.emplace_back ("b");
        // c -> a; c -> b;
        graph.emplace_back ("c").adjacent ({&va, &vb});

        auto const h1 = hash_vertices (std::begin (graph), std::end (graph));
        auto const h2 = hash_vertices (std::rbegin (graph), std::rend (graph));

        auto const & mh1 = std::get<memoized_hashes> (h1);
        assert (mh1.size () == 3U);
        auto const & vdp1 = std::get<std::vector<vertex_digest_pair>> (h1);

        auto const & mh2 = std::get<memoized_hashes> (h2);
        assert (mh2.size () == 3U);
        auto const & vdp2 = std::get<std::vector<vertex_digest_pair>> (h2);

        assert (
            std::equal (std::begin (vdp1), std::end (vdp1), std::begin (vdp2), std::end (vdp2)));
    }

    /// digraph G {
    ///    a -> b;
    ///    b -> a;
    /// }
    void tiny_loop () {
        std::list<vertex> graph;
        auto & va = graph.emplace_back ("a");
        auto & vb = graph.emplace_back ("b");

        va.adjacent (&vb); // a -> b;
        vb.adjacent (&va); // b -> a;

        auto const h1 = hash_vertices (std::begin (graph), std::end (graph));
        auto const h2 = hash_vertices (std::rbegin (graph), std::rend (graph));

        auto const & mh1 = std::get<memoized_hashes> (h1);
        assert (mh1.size () == 0U);
        auto const & vdp1 = std::get<std::vector<vertex_digest_pair>> (h1);

        auto const & mh2 = std::get<memoized_hashes> (h2);
        assert (mh2.size () == 0U);
        auto const & vdp2 = std::get<std::vector<vertex_digest_pair>> (h2);

        assert (
            std::equal (std::begin (vdp1), std::end (vdp1), std::begin (vdp2), std::end (vdp2)));
    }

    /// digraph G {
    ///    a -> b;
    ///    b -> a;
    ///    c -> a;
    ///    d -> c;
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
        std::list<vertex> graph;
        auto & va = graph.emplace_back ("a");
        auto & vb = graph.emplace_back ("b");
        auto & vc = graph.emplace_back ("c");
        auto & vd = graph.emplace_back ("d");

        va.adjacent (&vb); // a -> b;
        vb.adjacent (&va); // b -> a;
        vc.adjacent (&va); // c -> a;
        vd.adjacent (&vc); // d -> c;

        auto const h1 = hash_vertices (std::begin (graph), std::end (graph));
        auto const h2 = hash_vertices (std::rbegin (graph), std::rend (graph));

        auto const mh1 = std::move (std::get<memoized_hashes> (h1));
        assert (mh1.size () == 2);
        auto const vdp1 = std::move (std::get<std::vector<vertex_digest_pair>> (h1));

        auto const mh2 = std::move (std::get<memoized_hashes> (h2));
        assert (mh2.size () == 2);
        auto const vdp2 = std::move (std::get<std::vector<vertex_digest_pair>> (h2));

        assert (
            std::equal (std::begin (vdp1), std::end (vdp1), std::begin (vdp2), std::end (vdp2)));
    }


    /// digraph G {
    ///     c -> b -> a -> c;
    ///     f -> e -> d -> f;
    ///     g -> c;
    ///     g -> f;
    /// }
    void two_loops () {
        std::list<vertex> graph;
        auto & va = graph.emplace_back ("a");
        auto & vb = graph.emplace_back ("b").adjacent (&va); // b -> a
        auto & vc = graph.emplace_back ("c").adjacent (&vb); // c -> b
        va.adjacent (&vc);                                   // a -> c

        auto & vd = graph.emplace_back ("d");
        auto & ve = graph.emplace_back ("e").adjacent (&vd); // e -> d
        auto & vf = graph.emplace_back ("f").adjacent (&ve); // f -> e
        vd.adjacent (&vf);                                   // d -> f

        graph.emplace_back ("g").adjacent ({&vc, &vf});

        auto const h1 = hash_vertices (std::begin (graph), std::end (graph));
        auto const h2 = hash_vertices (std::rbegin (graph), std::rend (graph));

        auto const mh1 = std::move (std::get<memoized_hashes> (h1));
        assert (mh1.size () == 1U);
        auto const vdp1 = std::move (std::get<std::vector<vertex_digest_pair>> (h1));

        auto const mh2 = std::move (std::get<memoized_hashes> (h2));
        assert (mh2.size () == 1U);
        auto const vdp2 = std::move (std::get<std::vector<vertex_digest_pair>> (h2));

        assert (
            std::equal (std::begin (vdp1), std::end (vdp1), std::begin (vdp2), std::end (vdp2)));
    }

} // end anonymous namespace

int main () {
    std::cout << std::boolalpha;
#if 1
    std::cout << "Simple test:\n";
    test1 ();
    std::cout << "Bytes hashed so far: " << std::dec << hash::total () << '\n';

    std::cout << "Tiny loop test:\n";
    tiny_loop ();
    std::cout << "Bytes hashed so far: " << std::dec << hash::total () << '\n';
#endif

    std::cout << "Test with loop:\n";
    test_looped ();
    std::cout << "Total bytes hashed: " << std::dec << hash::total () << '\n';

    std::cout << "Two loops:\n";
    two_loops ();
    std::cout << "Bytes hashed: " << std::dec << hash::total () << '\n';
}
