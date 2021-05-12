#include "grid/sampling/refinement.h"
#include "grid/sampling/vector_sampling.h"
#include "tests/geometry/helpers.h"

#include <catch2/catch.hpp>

// TODO: Shrink test with boundary conditions
TEST_CASE("vector sampling refinement")
{
    VectorSampling<Location> sampling{Box{{0, 1, 0}, 4.}, 16, Location::Outer};

    sampling.find(Coordinates({2,7,11}))->value = Location::Boundary;
    sampling.find(Coordinates({2,7,12}))->value = Location::Boundary;
    sampling.find(Coordinates({2,8,11}))->value = Location::Inner;
    sampling.find(Coordinates({14,10,3}))->value = Location::Inner;
    sampling.find(Coordinates({14,12,3}))->value = Location::Inner;

    auto refinement = refine(sampling);

    REQUIRE(
        refinement.rasterSize() == (sampling.rasterSize() * 2).eval());
    REQUIRE(
        pointsEqual(refinement.toGlobal(Coordinates::constant(0)),
        sampling.toGlobal(Coordinates::constant(0))));
    REQUIRE(
        pointsEqual(refinement.toGlobal(refinement.rasterSize()),
        sampling.toGlobal(sampling.rasterSize())));

    const auto rasterCenter = Point{2., 3., 0.};
    REQUIRE(
        pointsEqual(refinement.toLocal(rasterCenter),
        (sampling.toLocal(rasterCenter) * 2).eval()));

    size_t boundaryCount = 0;
    size_t innerCount = 0;
    size_t outerCount = 0;
    refinement.voxels().process([&](const auto& element) {
        if (element.value == Location::Boundary) {
            ++boundaryCount;
        } else if (element.value == Location::Inner) {
            ++innerCount;
        } else {
            ++ outerCount;
        }
    });
    REQUIRE(boundaryCount == 40);
    REQUIRE(innerCount == 0);
    REQUIRE(outerCount == 0);
}
