#include <algorithm>
#include <cassert>
#include <iostream>
#include <list>

#include "hash.hpp"
#include "memhash.hpp"
#include "vertex.hpp"

namespace {

    using vertex_digest_pair = std::tuple<vertex const *, hash::digest>;
    using graph_digests = std::vector<vertex_digest_pair>;

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

    /// \tparam Iterator An iterator type which will produce an instance of type vertex.
    template <typename Iterator>
    std::tuple<memoized_hashes, graph_digests> hash_vertices (Iterator first, Iterator last) {
        graph_digests digests;
        memoized_hashes table;

        std::for_each (first, last, [&digests, &table] (vertex const & v) {
            digests.emplace_back (&v, vertex_hash (&v, &table));
        });

        // Sort the result by vertex name. This means that the iteration order won't have any
        // effect on the output.
        using value_type = graph_digests::value_type;
        std::sort (std::begin (digests), std::end (digests),
                   [] (value_type const & x, value_type const & y) {
                       return std::get<0> (x)->name () < std::get<0> (y)->name ();
                   });
        dump (std::cout, std::begin (digests), std::end (digests));
        return {std::move (table), std::move (digests)};
    }

    /// digraph G {
    ///    c -> a;
    ///    c -> b;
    /// }
    void simple_test () {
        std::list<vertex> graph;
        auto & va = graph.emplace_back ("a");
        auto & vb = graph.emplace_back ("b");
        auto & vc = graph.emplace_back ("c").adjacent ({&va, &vb}); // c -> a; c -> b;

        auto const h1 = hash_vertices (std::begin (graph), std::end (graph));

        auto const & mh1 = std::get<memoized_hashes> (h1);
        assert (mh1.size () == 3U);
        assert (mh1.count (&va) == 1U);
        assert (mh1.count (&vb) == 1U);
        assert (mh1.count (&vc) == 1U);

        auto const h2 = hash_vertices (std::rbegin (graph), std::rend (graph));

        auto const & mh2 = std::get<memoized_hashes> (h2);
        assert (mh2.size () == 3U);
        assert (mh2.count (&va) == 1U);
        assert (mh2.count (&vb) == 1U);
        assert (mh2.count (&vc) == 1U);

        auto const & vdp1 = std::get<graph_digests> (h1);
        auto const & vdp2 = std::get<graph_digests> (h2);
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
        auto & vb = graph.emplace_back ("b").adjacent (&va); // b -> a;
        va.adjacent (&vb); // a -> b;

        auto const h1 = hash_vertices (std::begin (graph), std::end (graph));
        assert (std::get<memoized_hashes> (h1).size () == 0U);

        auto const h2 = hash_vertices (std::rbegin (graph), std::rend (graph));
        assert (std::get<memoized_hashes> (h2).size () == 0U);

        auto const & vdp1 = std::get<graph_digests> (h1);
        auto const & vdp2 = std::get<graph_digests> (h2);
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
    ///
    /// | Name | |
    /// | "a" | Va/Vb/R0<br>
    ///         Vertex "a" -> vertex "b" -> loop back to the first (zeroth) vertex. The slash in
    ///         this notation can be thought of a directed edge between the two adjacent vertices.
    /// | "b" | Vb/Va/R0 |
    /// | "c" | Vc/Va/Vb/R1 |
    /// | "d" | Vd/Vc/Va/Vb/R2 |
    void test_looped () {
        std::list<vertex> graph;
        auto & va = graph.emplace_back ("a");
        auto & vb = graph.emplace_back ("b").adjacent (&va); // b -> a;
        va.adjacent (&vb); // a -> b;
        auto & vc = graph.emplace_back ("c").adjacent (&va); // c -> a;
        graph.emplace_back ("d").adjacent (&vc);             // d -> c;

        auto const forward_result = hash_vertices (std::begin (graph), std::end (graph));
        assert (std::get<memoized_hashes> (forward_result).size () == 0);

        auto const reverse_result = hash_vertices (std::rbegin (graph), std::rend (graph));
        assert (std::get<memoized_hashes> (reverse_result).size () == 0);

        auto const & vdp1 = std::get<graph_digests> (forward_result);
        auto const & vdp2 = std::get<graph_digests> (reverse_result);
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
        graph.emplace_back ("g").adjacent ({&vc, &vf});      // g -> c; g -> f;

        auto const h1 = hash_vertices (std::begin (graph), std::end (graph));
        assert (std::get<memoized_hashes> (h1).size () == 0U);

        auto const h2 = hash_vertices (std::rbegin (graph), std::rend (graph));
        assert (std::get<memoized_hashes> (h2).size () == 0U);

        auto const & vdp1 = std::get<graph_digests> (h1);
        auto const & vdp2 = std::get<graph_digests> (h2);
        assert (
            std::equal (std::begin (vdp1), std::end (vdp1), std::begin (vdp2), std::end (vdp2)));
    }

    /// digraph G {
    ///     a -> b;
    ///     b -> a;
    ///     c -> d;
    /// }
    void two_islands () {
        std::list<vertex> graph;
        auto & va = graph.emplace_back ("a");
        auto & vb = graph.emplace_back ("b").adjacent (&va);
        va.adjacent (&vb);
        auto & vc = graph.emplace_back ("c");
        auto & vd = graph.emplace_back ("d").adjacent (&vc);

        auto const h1 = hash_vertices (std::begin (graph), std::end (graph));
        auto const & mh1 = std::get<memoized_hashes> (h1);
        assert (mh1.size () == 2U);
        assert (mh1.count (&vc) == 1U);
        assert (mh1.count (&vd) == 1U);

        auto const h2 = hash_vertices (std::rbegin (graph), std::rend (graph));
        auto const & mh2 = std::get<memoized_hashes> (h2);
        assert (mh2.size () == 2U);
        assert (mh2.count (&vc) == 1U);
        assert (mh2.count (&vd) == 1U);

        auto const & vdp1 = std::get<graph_digests> (h1);
        auto const & vdp2 = std::get<graph_digests> (h2);
        assert (
            std::equal (std::begin (vdp1), std::end (vdp1), std::begin (vdp2), std::end (vdp2)));
    }

} // end anonymous namespace

int main () {
    std::cout << "Simple test:\n";
    simple_test ();
    std::cout << "Bytes hashed so far: " << std::dec << hash::total () << '\n';

    std::cout << "Tiny loop test:\n";
    tiny_loop ();
    std::cout << "Bytes hashed so far: " << std::dec << hash::total () << '\n';

    std::cout << "Test with loop:\n";
    test_looped ();
    std::cout << "Total bytes hashed: " << std::dec << hash::total () << '\n';

    std::cout << "Two loops:\n";
    two_loops ();
    std::cout << "Bytes hashed: " << std::dec << hash::total () << '\n';

    std::cout << "Two islands:\n";
    two_islands ();
    std::cout << "Bytes hashed: " << std::dec << hash::total () << '\n';
}
