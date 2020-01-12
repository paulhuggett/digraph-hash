#include <array>
#include <iostream>
#include <optional>
#include <unordered_map>

#include "hash.hpp"
#include "memhash.hpp"
#include "vertex.hpp"

namespace {

    using vertex_digest_pair = std::tuple<vertex const *, hash::digest>;

    // Iterator is an STL iterator type which will produce an instance of type pair.
    template <typename Iterator>
    void dump (std::ostream & os, Iterator first, Iterator last) {
        char const * separator = "";
        std::for_each (first, last, [&] (auto const & t) {
            os << separator << std::get<vertex const *> (t)->name << ':' << std::hex
               << std::get<hash::digest> (t);
            separator = " ";
        });
        os << '\n';
    }

    // Iterator is an STL iterator type which will produce an instance of type pair.
    template <typename Iterator>
    std::vector<vertex_digest_pair> make_hashes (Iterator first, Iterator last) {
        std::vector<vertex_digest_pair> digests;
        memoized_hashes table;
        std::for_each (first, last, [&digests, &table] (vertex const & v) {
            digests.emplace_back (&v, vertex_hash (&v, &table));
        });

        std::sort (std::begin (digests), std::end (digests),
                   [] (vertex_digest_pair const & x, vertex_digest_pair const & y) {
                       return std::get<0> (x)->name < std::get<0> (y)->name;
                   });
        dump (std::cout, std::begin (digests), std::end (digests));
        return digests;
    }

    void test1 () {
        // digraph G {
        //    c -> a;
        //    c -> b;
        // }
        enum { va, vb, vc };
        std::array<vertex, 3> graph{{vertex{"a"}, vertex{"b"}, vertex{"c"}}};
        graph[vc].add_dependents ({&graph[va], &graph[vb]});

        auto const h1 = make_hashes (std::begin (graph), std::end (graph));
        auto const h2 = make_hashes (std::rbegin (graph), std::rend (graph));
        assert (std::equal (std::begin (h1), std::end (h1), std::begin (h2), std::end (h2)));
    }

    void test_looped () {
        // digraph G {
        //    a -> b;
        //    b -> a;
        //    c -> a;
        // }
        enum { va, vb, vc };
        std::array<vertex, 3> graph{{vertex{"a"}, vertex{"b"}, vertex{"c"}}};
        graph[va].add_dependents ({&graph[vb]});
        graph[vb].add_dependents ({&graph[va]});
        graph[vc].add_dependents ({&graph[va]});

        auto const h1 = make_hashes (std::begin (graph), std::end (graph));
        auto const h2 = make_hashes (std::rbegin (graph), std::rend (graph));
        assert (std::equal (std::begin (h1), std::end (h1), std::begin (h2), std::end (h2)));
    }

} // end anonymous namespace

int main () {
    std::cout << "simple test:\n";
    test1 ();
    std::cout << "test with loop:\n";
    test_looped ();
}
