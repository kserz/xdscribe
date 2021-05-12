#include "utility/subsets.h"

#include <catch2/catch.hpp>

#include <numeric>
#include <set>
#include <vector>

size_t subsetsCount(size_t superSetSize, size_t subSetSize)
{
    std::vector<size_t> superset(superSetSize);
    std::iota(superset.begin(), superset.end(), 0);

    size_t generatedCount = 0;
    std::set<std::vector<size_t>> differentSets;
    subSets<size_t>(&superset, subSetSize).process([&] (const auto& subSet) {
        differentSets.insert(subSet);
        ++generatedCount;
    });
    REQUIRE(generatedCount == differentSets.size());
    return generatedCount;
}

TEST_CASE("subsets")
{
    REQUIRE(subsetsCount(5, 2) == 10);
    REQUIRE(subsetsCount(8, 3) == 56);
}
