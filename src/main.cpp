#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <list>

#include "hash.hpp"
#include "memhash.hpp"
#include "vertex.hpp"

namespace {

    using vertex_digest_pair = std::tuple<vertex const *, hash::digest>;
    using graph_digests = std::vector<vertex_digest_pair>;

} // end anonymous namespace

int main () {
    /// digraph G {
    ///     a -> b;
    ///     b -> a;
    ///     c -> d;
    /// }
    std::list<vertex> graph;
    vertex & va = graph.emplace_back ("a");
    vertex const & vb = graph.emplace_back ("b").adjacent (&va);
    va.adjacent (&vb);
    vertex const & vc = graph.emplace_back ("c");
    graph.emplace_back ("d").adjacent (&vc);

    graph_digests digests;
    memoized_hashes table;
    std::transform (std::begin (graph), std::end (graph),
                    std::inserter (digests, std::end (digests)), [&table] (vertex const & v) {
                        return vertex_digest_pair{&v, vertex_hash (&v, &table)};
                    });

    std::for_each (std::begin (digests), std::end (digests), [] (auto const & vdp) {
        std::cout << std::get<vertex const *> (vdp)->name () << ':' << std::hex
                  << std::get<hash::digest> (vdp) << '\n';
    });
}
