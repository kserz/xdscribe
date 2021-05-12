#include "grid/sampling/sampling.h"
#include "grid/sampling/vector_sparse_raster.h"
#include "grid/sampling/xd_iterator.h"

#include <catch2/catch.hpp>

#include <set>
#include <functional>

TEST_CASE("xd iterator")
{
    int counter = 0;
    XDIterator<1>::run(
        Coordinates({10, 4, 3}), [&] (const Coordinates&)
    {
        ++counter;
    });
    REQUIRE(counter == 10);

    std::set<
            Coordinates,
            std::function<bool(const Coordinates&,const Coordinates&)>>
        img(&preceding);
    XDIterator<DIMS>::run(
        Coordinates({10, 6, 4}), [&] (const Coordinates& coords)
    {
        img.insert(coords);
    });
    REQUIRE(img.size() == 240);

    int imgCounter = 0;
    counter = 0;
    XDIterator<>::run(
        Coordinates({10, 6, 5}), [&] (const Coordinates& coords)
    {
        ++counter;
        if (img.count(coords) > 0) {
            ++imgCounter;
        }
    });
    REQUIRE(counter == 300);
    REQUIRE(imgCounter == 240);
}
