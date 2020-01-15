#include <list>

#include <algorithm>
#include <iterator>
#include <functional>
#include <set>

#include <gmock/gmock.h>

#include "hash.hpp"
#include "memhash.hpp"
#include "vertex.hpp"

using testing::Eq;
using testing::UnorderedElementsAre;

namespace std {

    template <>
    struct hash<vertex const *> {
        size_t operator() (vertex const * const v) const noexcept {
            return hash<std::string>{}(v->name ());
        }
    };

} // namespace std

namespace {

    using vertex_digest_pair = std::tuple<vertex const *, hash::digest>;
    using graph_digests = std::vector<vertex_digest_pair>;

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
        return {std::move (table), std::move (digests)};
    }

    template <typename Iterator>
    auto keys (Iterator first, Iterator last) {
        std::set<vertex const *> result;
        std::transform (
            first, last, std::inserter (result, result.end ()),
            [] (typename std::iterator_traits<Iterator>::value_type const & v) { return v.first; });
        return result;
    }

} // end anonymous namespace

// Test behavior with a graph of the form:
//
//     digraph G {
//        c -> a;
//        c -> b;
//     }
TEST (DigraphHash, Simple) {
    std::list<vertex> graph;
    vertex const & va = graph.emplace_back ("a");
    vertex const & vb = graph.emplace_back ("b");
    vertex const & vc = graph.emplace_back ("c").adjacent ({&va, &vb}); // c -> a; c -> b;

    auto const expected_cache = UnorderedElementsAre (&va, &vb, &vc);

    // Forward
    auto const h1 = hash_vertices (std::begin (graph), std::end (graph));
    auto const & mh1 = std::get<memoized_hashes> (h1);
    EXPECT_THAT (keys (std::begin (mh1), std::end (mh1)), expected_cache);

    // Reverse
    auto const h2 = hash_vertices (std::rbegin (graph), std::rend (graph));
    auto const & mh2 = std::get<memoized_hashes> (h2);
    EXPECT_THAT (keys (std::begin (mh2), std::end (mh2)), expected_cache);

    // Check that digests are the same for the two traversal orders.
    EXPECT_THAT (std::get<graph_digests> (h1), Eq (std::get<graph_digests> (h2)))
        << "Output should not be affected by traversal order";
}

/// digraph G {
///    a -> b -> a;
/// }
TEST (DigraphHash, TinyLoop) {
    std::list<vertex> graph;
    vertex & va = graph.emplace_back ("a");
    vertex const & vb = graph.emplace_back ("b").adjacent (&va); // b -> a;
    va.adjacent (&vb);                                   // a -> b;

    // Forward
    auto const h1 = hash_vertices (std::begin (graph), std::end (graph));
    EXPECT_EQ (std::get<memoized_hashes> (h1).size (), 0U)
        << "Expected nothing to be memoized for this graph";

    // Reverse
    auto const h2 = hash_vertices (std::rbegin (graph), std::rend (graph));
    EXPECT_EQ (std::get<memoized_hashes> (h2).size (), 0U)
        << "Expected nothing to be memoized for this graph";

    // Check that digests are the same for the two traversal orders.
    EXPECT_THAT (std::get<graph_digests> (h1), Eq (std::get<graph_digests> (h2)))
        << "Output should not be affected by traversal order";
}

/// digraph G {
///    d -> c -> a -> b -> a;
/// }
TEST (DigraphHash, Loop2) {
    std::list<vertex> graph;
    vertex & va = graph.emplace_back ("a");
    vertex const & vb = graph.emplace_back ("b").adjacent (&va); // b -> a;
    va.adjacent (&vb);                                   // a -> b;
    vertex const & vc = graph.emplace_back ("c").adjacent (&va); // c -> a;
    graph.emplace_back ("d").adjacent (&vc);             // d -> c;

    // Forward
    auto const forward_result = hash_vertices (std::begin (graph), std::end (graph));
    EXPECT_EQ (std::get<memoized_hashes> (forward_result).size (), 0U)
        << "Expected nothing to be memoized for this graph";

    // Reverse
    auto const reverse_result = hash_vertices (std::rbegin (graph), std::rend (graph));
    EXPECT_EQ (std::get<memoized_hashes> (reverse_result).size (), 0U)
        << "Expected nothing to be memoized for this graph";

    // Check that digests are the same for the two traversal orders.
    EXPECT_THAT (std::get<graph_digests> (forward_result),
                 Eq (std::get<graph_digests> (reverse_result)))
        << "Output should not be affected by traversal order";
}

/// digraph G {
///     g -> c -> b -> a -> c;
///     g -> f -> e -> d -> f;
/// }
TEST (DigraphHash, TwoLoops) {
    std::list<vertex> graph;
    vertex & va = graph.emplace_back ("a");
    vertex const & vb = graph.emplace_back ("b").adjacent (&va); // b -> a
    vertex const & vc = graph.emplace_back ("c").adjacent (&vb); // c -> b
    va.adjacent (&vc);                                   // a -> c
    vertex & vd = graph.emplace_back ("d");
    vertex const & ve = graph.emplace_back ("e").adjacent (&vd); // e -> d
    vertex const & vf = graph.emplace_back ("f").adjacent (&ve); // f -> e
    vd.adjacent (&vf);                                   // d -> f
    graph.emplace_back ("g").adjacent ({&vc, &vf});      // g -> c; g -> f;

    // Forward
    auto const h1 = hash_vertices (std::begin (graph), std::end (graph));
    EXPECT_EQ (std::get<memoized_hashes> (h1).size (), 0U)
        << "Expected nothing to be memoized for this graph";

    // Reverse
    auto const h2 = hash_vertices (std::rbegin (graph), std::rend (graph));
    EXPECT_EQ (std::get<memoized_hashes> (h2).size (), 0U)
        << "Expected nothing to be memoized for this graph";

    // Check that digests are the same for the two traversal orders.
    EXPECT_THAT (std::get<graph_digests> (h1), Eq (std::get<graph_digests> (h2)))
        << "Output should not be affected by traversal order";
}

/// digraph G {
///     a -> b -> a;
///     c -> d;
/// }
TEST (DigraphHash, TwoIslands) {
    std::list<vertex> graph;
    vertex & va = graph.emplace_back ("a");
    vertex const & vb = graph.emplace_back ("b").adjacent (&va);
    va.adjacent (&vb);
    vertex const & vc = graph.emplace_back ("c");
    vertex const & vd = graph.emplace_back ("d").adjacent (&vc);

    auto const expected_cache = UnorderedElementsAre (&vc, &vd);

    // Forward
    auto const h1 = hash_vertices (std::begin (graph), std::end (graph));
    {
        auto const & mh1 = std::get<memoized_hashes> (h1);
        EXPECT_THAT (keys (std::begin (mh1), std::end (mh1)), expected_cache);
    }

    // Reverse
    auto const h2 = hash_vertices (std::rbegin (graph), std::rend (graph));
    {
        auto const & mh2 = std::get<memoized_hashes> (h2);
        EXPECT_THAT (keys (std::begin (mh2), std::end (mh2)), expected_cache);
    }
    // Check that digests are the same for the two traversal orders.
    EXPECT_THAT (std::get<graph_digests> (h1), Eq (std::get<graph_digests> (h2)))
        << "Output should not be affected by traversal order";
}
