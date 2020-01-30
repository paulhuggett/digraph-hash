#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <list>

#include "hash.hpp"
#include "memhash.hpp"
#include "vertex.hpp"

int main () {
    /// digraph G {
    ///     a -> b;
    ///     b -> a;
    ///     c -> d;
    /// }
    std::list<vertex> graph;
    vertex & va = graph.emplace_back ("a");
    vertex const & vb = graph.emplace_back ("b").add_edge (&va);
    va.add_edge (&vb);
    vertex const & vc = graph.emplace_back ("c");
    graph.emplace_back ("d").add_edge (&vc);

    using vertex_digest_pair = std::tuple<vertex const *, hash::digest>;
    std::vector<vertex_digest_pair> digests;
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
