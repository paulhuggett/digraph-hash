#include "memhash.hpp"

#include <algorithm>
#include <functional>
#include <iterator>
#include <list>
#include <set>

#include <gmock/gmock.h>

#include "config.hpp"
#include "hash.hpp"
#include "vertex.hpp"

using namespace std::string_literals;

using testing::ElementsAre;
using testing::Eq;
using testing::UnorderedElementsAre;

#ifdef FNV1_HASH_ENABLED
#    define STRING_HASH_EXPECT_THAT(value, matcher)
#    define STRING_HASH_EXPECT_EQ(val1, val2)
#else
#    define STRING_HASH_EXPECT_THAT(value, matcher) EXPECT_THAT (value, matcher)
#    define STRING_HASH_EXPECT_EQ(val1, val2) EXPECT_EQ (val1, val2)
#endif // FNV1_HASH_ENABLED

namespace {

    using graph_digests = std::vector<std::tuple<vertex const *, hash::digest>>;

    /// \tparam Iterator An iterator type which will produce an instance of type vertex.
    template <typename Iterator>
    auto hash_vertices (Iterator first, Iterator last)
        -> std::tuple<memoized_hashes, graph_digests> {
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
        std::transform (first, last, std::inserter (result, result.end ()),
                        [] (auto const & v) { return v.first; });
        return result;
    }

    template <typename Container>
    auto keys (Container const & c) {
        return keys (std::begin (c), std::end (c));
    }

} // end anonymous namespace

std::ostream & operator<< (std::ostream & os, vertex const * const v) {
    return os << *v;
}

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
    vertex const & vc = graph.emplace_back ("c").add_edge ({&va, &vb}); // c -> a; c -> b;

    auto const expected_cache = UnorderedElementsAre (&va, &vb, &vc);
#ifndef FNV1_HASH_ENABLED
    auto const expected_digests =
        ElementsAre (std::make_tuple (&va, "VaE"s), std::make_tuple (&vb, "VbE"s),
                     std::make_tuple (&vc, "Vc/VaE/VbEE"s));
#endif

    // Forward
    auto const forward_result = hash_vertices (std::begin (graph), std::end (graph));
    EXPECT_THAT (keys (std::get<memoized_hashes> (forward_result)), expected_cache);
    STRING_HASH_EXPECT_THAT (std::get<graph_digests> (forward_result), expected_digests);

    // Reverse
    auto const reverse_result = hash_vertices (std::rbegin (graph), std::rend (graph));
    EXPECT_THAT (keys (std::get<memoized_hashes> (reverse_result)), expected_cache);
    STRING_HASH_EXPECT_THAT (std::get<graph_digests> (reverse_result), expected_digests);

    // Check that digests are the same for the two traversal orders.
    EXPECT_THAT (std::get<graph_digests> (forward_result),
                 Eq (std::get<graph_digests> (reverse_result)))
        << "Output should not be affected by traversal order";
}

// Test behavior with a graph of the form:
//
//     digraph G {
//         a -> b -> b;
//     }
TEST (DigraphHash, SelfLoop) {
    std::list<vertex> graph;
    vertex & va = graph.emplace_back ("a");
    vertex & vb = graph.emplace_back ("b");
    va.add_edge (&vb); // a -> b;
    vb.add_edge (&vb); // b -> b;

    auto const expected_cache = UnorderedElementsAre (&va, &vb);
#ifndef FNV1_HASH_ENABLED
    auto const expected_digests =
        ElementsAre (std::make_tuple (&va, "Va/Vb/R0EE"s), std::make_tuple (&vb, "Vb/R0E"s));
#endif

    // Forward
    auto const forward_result = hash_vertices (std::begin (graph), std::end (graph));
    EXPECT_THAT (keys (std::get<memoized_hashes> (forward_result)), expected_cache);
    STRING_HASH_EXPECT_THAT (std::get<graph_digests> (forward_result), expected_digests);

    // Reverse
    auto const reverse_result = hash_vertices (std::rbegin (graph), std::rend (graph));
    EXPECT_THAT (keys (std::get<memoized_hashes> (forward_result)), expected_cache);
    STRING_HASH_EXPECT_THAT (std::get<graph_digests> (reverse_result), expected_digests);

    // Check that digests are the same for the two traversal orders.
    EXPECT_THAT (std::get<graph_digests> (forward_result),
                 Eq (std::get<graph_digests> (reverse_result)))
        << "Output should not be affected by traversal order";
}


// Test behavior with a graph of the form:
//
//     digraph G {
//         a -> b -> a;
//     }
TEST (DigraphHash, TinyLoop) {
    std::list<vertex> graph;
    vertex & va = graph.emplace_back ("a");
    vertex const & vb = graph.emplace_back ("b").add_edge (&va); // b -> a;
    va.add_edge (&vb);                                           // a -> b;

#ifndef FNV1_HASH_ENABLED
    auto const expected_digests =
        ElementsAre (std::make_tuple (&va, "Va/Vb/R1EE"s), std::make_tuple (&vb, "Vb/Va/R1EE"s));
#endif

    // Forward
    auto const forward_result = hash_vertices (std::begin (graph), std::end (graph));
    EXPECT_EQ (std::get<memoized_hashes> (forward_result).size (), 0U)
        << "Expected nothing to be memoized for this graph";
    STRING_HASH_EXPECT_THAT (std::get<graph_digests> (forward_result), expected_digests);

    // Reverse
    auto const reverse_result = hash_vertices (std::rbegin (graph), std::rend (graph));
    EXPECT_EQ (std::get<memoized_hashes> (reverse_result).size (), 0U)
        << "Expected nothing to be memoized for this graph";
    STRING_HASH_EXPECT_THAT (std::get<graph_digests> (reverse_result), expected_digests);

    // Check that digests are the same for the two traversal orders.
    EXPECT_THAT (std::get<graph_digests> (forward_result),
                 Eq (std::get<graph_digests> (reverse_result)))
        << "Output should not be affected by traversal order";
}

// Test behavior with a graph of the form:
//
//     digraph G {
//         d -> c -> a -> b -> a;
//     }
TEST (DigraphHash, Loop2) {
    std::list<vertex> graph;
    vertex & va = graph.emplace_back ("a");
    vertex const & vb = graph.emplace_back ("b").add_edge (&va); // b -> a;
    va.add_edge (&vb);                                           // a -> b;
    vertex const & vc = graph.emplace_back ("c").add_edge (&va); // c -> a;
    vertex const & vd = graph.emplace_back ("d").add_edge (&vc); // d -> c;

    auto const expected_cache = UnorderedElementsAre (&vc, &vd);
#ifndef FNV1_HASH_ENABLED
    auto const expected_digests = ElementsAre (
        std::make_tuple (&va, "Va/Vb/R1EE"s), std::make_tuple (&vb, "Vb/Va/R1EE"s),
        std::make_tuple (&vc, "Vc/Va/Vb/R1EEE"s), std::make_tuple (&vd, "Vd/Vc/Va/Vb/R1EEEE"s));
#endif

    // Forward
    auto const forward_result = hash_vertices (std::begin (graph), std::end (graph));
    EXPECT_THAT (keys (std::get<memoized_hashes> (forward_result)), expected_cache);
    STRING_HASH_EXPECT_THAT (std::get<graph_digests> (forward_result), expected_digests);

    // Reverse
    auto const reverse_result = hash_vertices (std::rbegin (graph), std::rend (graph));
    EXPECT_THAT (keys (std::get<memoized_hashes> (reverse_result)), expected_cache);
    STRING_HASH_EXPECT_THAT (std::get<graph_digests> (reverse_result), expected_digests);

    // Check that digests are the same for the two traversal orders.
    EXPECT_THAT (std::get<graph_digests> (forward_result),
                 Eq (std::get<graph_digests> (reverse_result)))
        << "Output should not be affected by traversal order";
}

// Test behavior with a graph of the form:
//
//     digraph G {
//         g -> c -> b -> a -> c;
//         g -> f -> e -> d -> f;
//     }
TEST (DigraphHash, TwoLoops) {
    std::list<vertex> graph;
    vertex & va = graph.emplace_back ("a");
    vertex const & vb = graph.emplace_back ("b").add_edge (&va); // b -> a
    vertex const & vc = graph.emplace_back ("c").add_edge (&vb); // c -> b
    va.add_edge (&vc);                                           // a -> c
    vertex & vd = graph.emplace_back ("d");
    vertex const & ve = graph.emplace_back ("e").add_edge (&vd);        // e -> d
    vertex const & vf = graph.emplace_back ("f").add_edge (&ve);        // f -> e
    vd.add_edge (&vf);                                                  // d -> f
    vertex const & vg = graph.emplace_back ("g").add_edge ({&vc, &vf}); // g -> c; g -> f;

    auto const expected_cache = UnorderedElementsAre (&vg);
#ifndef FNV1_HASH_ENABLED
    auto const expected_digests = ElementsAre (
        std::make_tuple (&va, "Va/Vc/Vb/R2EEE"s), std::make_tuple (&vb, "Vb/Va/Vc/R2EEE"s),
        std::make_tuple (&vc, "Vc/Vb/Va/R2EEE"s), std::make_tuple (&vd, "Vd/Vf/Ve/R2EEE"s),
        std::make_tuple (&ve, "Ve/Vd/Vf/R2EEE"s), std::make_tuple (&vf, "Vf/Ve/Vd/R2EEE"s),
        std::make_tuple (&vg, "Vg/Vc/Vb/Va/R2EEE/Vf/Ve/Vd/R2EEEE"s));
#endif

    // Forward
    auto const forward_result = hash_vertices (std::begin (graph), std::end (graph));
    EXPECT_THAT (keys (std::get<memoized_hashes> (forward_result)), expected_cache);
    STRING_HASH_EXPECT_THAT (std::get<graph_digests> (forward_result), expected_digests);

    // Reverse
    auto const reverse_result = hash_vertices (std::rbegin (graph), std::rend (graph));
    EXPECT_THAT (keys (std::get<memoized_hashes> (reverse_result)), expected_cache);
    STRING_HASH_EXPECT_THAT (std::get<graph_digests> (reverse_result), expected_digests);

    // Check that digests are the same for the two traversal orders.
    EXPECT_THAT (std::get<graph_digests> (forward_result),
                 Eq (std::get<graph_digests> (reverse_result)))
        << "Output should not be affected by traversal order";
}

// Test behavior with a graph of the form:
//
//     digraph G {
//         a -> b -> a;
//         c -> d;
//     }
TEST (DigraphHash, TwoIslands) {
    std::list<vertex> graph;
    vertex & va = graph.emplace_back ("a");
    vertex const & vb = graph.emplace_back ("b").add_edge (&va);
    va.add_edge (&vb);
    vertex & vc = graph.emplace_back ("c");
    vertex const & vd = graph.emplace_back ("d");
    vc.add_edge (&vd);

    auto const expected_cache = UnorderedElementsAre (&vc, &vd);
#ifndef FNV1_HASH_ENABLED
    auto const expected_digests =
        ElementsAre (std::make_tuple (&va, "Va/Vb/R1EE"s), std::make_tuple (&vb, "Vb/Va/R1EE"s),
                     std::make_tuple (&vc, "Vc/VdEE"s), std::make_tuple (&vd, "VdE"s));
#endif

    // Forward
    auto const forward_result = hash_vertices (std::begin (graph), std::end (graph));
    EXPECT_THAT (keys (std::get<memoized_hashes> (forward_result)), expected_cache);
    STRING_HASH_EXPECT_THAT (std::get<graph_digests> (forward_result), expected_digests);

    // Reverse
    auto const reverse_result = hash_vertices (std::rbegin (graph), std::rend (graph));
    EXPECT_THAT (keys (std::get<memoized_hashes> (reverse_result)), expected_cache);
    STRING_HASH_EXPECT_THAT (std::get<graph_digests> (reverse_result), expected_digests);

    // Check that digests are the same for the two traversal orders.
    EXPECT_THAT (std::get<graph_digests> (forward_result),
                 Eq (std::get<graph_digests> (reverse_result)))
        << "Output should not be affected by traversal order";
}

// Test behavior with a graph of the form:
//
//     digraph G {
//         a -> b -> c -> b;
//         a -> d -> e;
//         d -> f;
//     }
TEST (DigraphHash, CyclicAndAcyclicPaths) {
    std::list<vertex> graph;
    vertex & va = graph.emplace_back ("a");
    vertex & vb = graph.emplace_back ("b");
    va.add_edge (&vb);
    vertex const & vc = graph.emplace_back ("c").add_edge (&vb);
    vb.add_edge (&vc);
    vertex & vd = graph.emplace_back ("d");
    va.add_edge (&vd);
    vertex const & ve = graph.emplace_back ("e");
    vd.add_edge (&ve);
    vertex const & vf = graph.emplace_back ("f");
    vd.add_edge (&vf);

    auto const result = hash_vertices (std::begin (graph), std::end (graph));
    EXPECT_THAT (keys (std::get<memoized_hashes> (result)),
                 UnorderedElementsAre (&va, &vd, &ve, &vf));
    STRING_HASH_EXPECT_THAT (
        std::get<graph_digests> (result),
        ElementsAre (std::make_tuple (&va, "Va/Vb/Vc/R1EE/Vd/VeE/VfEEE"s),
                     std::make_tuple (&vb, "Vb/Vc/R1EE"s), std::make_tuple (&vc, "Vc/Vb/R1EE"s),
                     std::make_tuple (&vd, "Vd/VeE/VfEE"s), std::make_tuple (&ve, "VeE"s),
                     std::make_tuple (&vf, "VfE"s)));
}


// Check that we can encode multiple edges between the same pair of vertices.
//
//     digraph G {
//         a -> b;
//         a -> b;
//     }
TEST (DigraphHash, TwoEdges) {
    std::list<vertex> graph;
    vertex & va = graph.emplace_back ("a");
    vertex & vb = graph.emplace_back ("b");
    va.add_edge (&vb);
    va.add_edge (&vb);
    {
        memoized_hashes ma;
        hash::digest const hva = vertex_hash (&va, &ma);
        STRING_HASH_EXPECT_EQ (hva, "Va/VbE/VbEE");
        EXPECT_THAT (keys (ma), UnorderedElementsAre (&va, &vb));
    }
    {
        memoized_hashes mb;
        hash::digest const hvb = vertex_hash (&vb, &mb);
        STRING_HASH_EXPECT_EQ (hvb, "VbE");
        EXPECT_THAT (keys (mb), UnorderedElementsAre (&vb));
    }
}

//     digraph G {
//         a -> b -> d;
//         a -> c -> d;
//     }
TEST (DigraphHash, Diamond) {
    std::list<vertex> graph;
    vertex & va = graph.emplace_back ("a");
    vertex & vb = graph.emplace_back ("b");
    vertex & vc = graph.emplace_back ("c");
    vertex & vd = graph.emplace_back ("d");
    va.add_edge (&vb);
    vb.add_edge (&vd);
    va.add_edge (&vc);
    vc.add_edge (&vd);

    {
        memoized_hashes ma;
        hash::digest const hva = vertex_hash (&va, &ma);
        STRING_HASH_EXPECT_EQ (hva, "Va/Vb/VdEE/Vc/VdEEE");
        EXPECT_THAT (keys (ma), UnorderedElementsAre (&va, &vb, &vc, &vd));
    }
    {
        memoized_hashes mb;
        hash::digest const hvb = vertex_hash (&vb, &mb);
        STRING_HASH_EXPECT_EQ (hvb, "Vb/VdEE");
        EXPECT_THAT (keys (mb), UnorderedElementsAre (&vb, &vd));
    }
    {
        memoized_hashes mc;
        hash::digest const hvb = vertex_hash (&vc, &mc);
        STRING_HASH_EXPECT_EQ (hvb, "Vc/VdEE");
        EXPECT_THAT (keys (mc), UnorderedElementsAre (&vc, &vd));
    }
    {
        memoized_hashes md;
        hash::digest const hvc = vertex_hash (&vd, &md);
        STRING_HASH_EXPECT_EQ (hvc, "VdE");
        EXPECT_THAT (keys (md), UnorderedElementsAre (&vd));
    }
}

// Test behaviour with a twice visited loop
//     digraph G {
//         a -> b -> c -> b;
//         a -> d -> b;
//     }
TEST (DigraphHash, TwiceVisitedLoop) {
    std::list<vertex> graph;
    vertex & va = graph.emplace_back ("a");
    vertex & vb = graph.emplace_back ("b");
    vertex & vc = graph.emplace_back ("c");
    vertex & vd = graph.emplace_back ("d");
    va.add_edge (&vb);
    vb.add_edge (&vc);
    vc.add_edge (&vb);
    va.add_edge (&vd);
    vd.add_edge (&vb);

    {
        memoized_hashes ma;
        hash::digest const hva = vertex_hash (&va, &ma);
        STRING_HASH_EXPECT_EQ (hva, "Va/Vb/Vc/R1EE/Vd/Vb/Vc/R1EEEE");
        EXPECT_THAT (keys (ma), UnorderedElementsAre (&va, &vd));
    }
    {
        memoized_hashes mb;
        hash::digest const hvb = vertex_hash (&vb, &mb);
        STRING_HASH_EXPECT_EQ (hvb, "Vb/Vc/R1EE");
        EXPECT_EQ (mb.size (), 0U);
    }
    {
        memoized_hashes mc;
        hash::digest const hvc = vertex_hash (&vc, &mc);
        STRING_HASH_EXPECT_EQ (hvc, "Vc/Vb/R1EE");
        EXPECT_EQ (mc.size (), 0U);
    }
    {
        memoized_hashes md;
        hash::digest const hvd = vertex_hash (&vd, &md);
        STRING_HASH_EXPECT_EQ (hvd, "Vd/Vb/Vc/R1EEE");
        EXPECT_THAT (keys (md), UnorderedElementsAre (&vd));
    }
}
