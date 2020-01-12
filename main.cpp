#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct vertex;

struct vertex {
    explicit vertex (char const * name_, std::initializer_list<vertex *> dependents_ = {})
            : name{name_}
            , dependents{dependents_} {}
    void add_dependents (std::initializer_list<vertex *> d) {
        dependents.insert (std::end (dependents), d);
    }

    std::string name;
    std::vector<vertex *> dependents;
};


class hash {
public:
#if 1
    using digest = std::string;
    digest finalize () const noexcept { return state_; }
#else
    using digest = uint64_t;
    digest finalize () const noexcept {
        // FNV-1a hash of the string (including a final null)
        static constexpr uint64_t fnv1a_64_init = 0xcbf29ce4'84222325U;
        static constexpr uint64_t fnv1a_64_prime = 0x00000100'000001b3U;

        auto const append = [] (uint8_t const v, uint64_t hval) noexcept {
            return (hval ^ static_cast<uint64_t> (v)) * fnv1a_64_prime;
        };

        auto result = fnv1a_64_init;
        for (auto s = reinterpret_cast<uint8_t const *> (state_.c_str ()); *s != '\0'; ++s) {
            result = append (*s, result);
        }
        return append ('\0', result);
    }
#endif

    void update (vertex const & x) { state_ += 'F' + x.name + '/'; }
    void update (size_t back_ref) { state_ += 'R' + std::to_string (back_ref) + '/'; }
    void update (digest const & d) { state_ += d; }

private:
    std::string state_;
};


using memoized_hashes = std::unordered_map<vertex const *, std::tuple<bool, hash::digest>>;
using visited = std::unordered_map<vertex const *, size_t>;

std::optional<hash::digest> vertex_hash_impl (vertex const * const v, memoized_hashes * const table,
                                              hash * const h, visited * const visited) {
    {
        // Have we computed the hash for this function already? If so, did it involve a loop?
        // If we have, and there was no loop, we can return the result immediately.
        auto const pos = table->find (v);
        if (pos != std::end (*table) && !std::get<bool> (pos->second)) {
            // std::cout << "Returning pre-computed hash for " << v->name << '\n';
            return {std::get<hash::digest> (pos->second)};
        }
    }
    {
        // Have we previously visited this vertex during this search? If so, add a back-reference
        // to the hash and tell the caller that this result should not be memoized.
        auto const back_ref_pos = visited->find (v);
        if (back_ref_pos != std::end (*visited)) {
            // std::cout << "Hashing back reference " << back_ref_pos->second << '\n';
            h->update (back_ref_pos->second);
            return {std::nullopt};
        }
    }
    (*visited)[v] = visited->size ();

    // std::cout << "Computing hash for " << v->name << '\n';
    hash h2;
    h2.update (*v);
    h->update (*v);
    bool looped = false;
    for (auto dependent : v->dependents) {
        if (auto const r = vertex_hash_impl (dependent, table, h, visited)) {
            h2.update (r.value ());
        } else {
            looped = true;
        }
    }
    auto const result = h2.finalize ();
    (*table)[v] = std::make_tuple (looped, result);
    if (looped) {
        return {std::nullopt};
    }
    return {result};
}


hash::digest vertex_hash (vertex const * const v, memoized_hashes * const table) {
    hash h;
    visited visited;
    std::optional<hash::digest> const result = vertex_hash_impl (v, table, &h, &visited);
    if (result.has_value ()) {
        return result.value ();
    }
    return h.finalize ();
}

template <size_t Size>
void dump (std::ostream & os, std::array<vertex, Size> const & graph,
           std::array<hash::digest, Size> const & digests) {
    auto gpos = std::begin (graph);
    auto const gend = std::end (graph);
    auto dpos = std::begin (digests);
    assert (std::distance (gpos, std::end (graph)) == std::distance (dpos, std::end (digests)));

    char const * separator = "";
    for (; gpos != gend; ++gpos, ++dpos) {
        os << separator << gpos->name << ':' << std::hex << *dpos;
        separator = " ";
    }
    os << '\n';
}

enum { va, vb, vc };
void test1 () {
    // digraph G {
    //    c -> a;
    //    c -> b;
    //}
    std::array<vertex, 3> graph{{vertex{"a"}, vertex{"b"}, vertex{"c"}}};
    graph[vc].add_dependents ({&graph[va], &graph[vb]});

    std::array<hash::digest, 3> h1{};

    {
        memoized_hashes table;
        h1[vc] = vertex_hash (&graph[vc], &table);
        h1[vb] = vertex_hash (&graph[vb], &table);
        h1[va] = vertex_hash (&graph[va], &table);
        dump (std::cout, graph, h1);
    }

    std::array<hash::digest, 3> h2{};
    {
        memoized_hashes table;
        h2[va] = vertex_hash (&graph[va], &table);
        h2[vb] = vertex_hash (&graph[vb], &table);
        h2[vc] = vertex_hash (&graph[vc], &table);
        dump (std::cout, graph, h2);
    }
    assert (std::equal (std::begin (h1), std::end (h1), std::begin (h2), std::end (h2)));
}

void test_looped () {
    // digraph G {
    //    a -> b;
    //    b -> a;
    //    c -> a;
    //}
    std::array<vertex, 3> graph{{vertex{"a"}, vertex{"b"}, vertex{"c"}}};
    graph[va].add_dependents ({&graph[vb]});
    graph[vb].add_dependents ({&graph[va]});
    graph[vc].add_dependents ({&graph[va]});

    std::array<hash::digest, 3> h1{};

    {
        memoized_hashes table;
        h1[va] = vertex_hash (&graph[va], &table);
        h1[vb] = vertex_hash (&graph[vb], &table);
        h1[vc] = vertex_hash (&graph[vc], &table);
        dump (std::cout, graph, h1);
    }

    std::array<hash::digest, 3> h2{};
    {
        memoized_hashes table;
        h2[vc] = vertex_hash (&graph[vc], &table);
        h2[vb] = vertex_hash (&graph[vb], &table);
        h2[va] = vertex_hash (&graph[va], &table);
        dump (std::cout, graph, h2);
    }
    assert (std::equal (std::begin (h1), std::end (h1), std::begin (h2), std::end (h2)));
}



int main () {
    std::cout << "test:\n";
    test1 ();
    std::cout << "test with loop:\n";
    test_looped ();
}
