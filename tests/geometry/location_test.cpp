#include "geometry/entity/polytope.h"
#include "geometry/location/location.h"

#include <catch2/catch.hpp>

TEST_CASE("point location")
{
    auto p = Polytope::loadObj("examples/tetrahedron_4.obj");
    const auto facets = p.facetGeometries();

    REQUIRE(locatePoint(Point::constant(0.), facets) == Location::Inner);
    REQUIRE(locatePoint(Point({0., 0.5, -1.}), facets) == Location::Inner);
    REQUIRE(locatePoint(Point({0., 0., 2.}), facets) == Location::Outer);
    REQUIRE(locatePoint(Point({0., 0., -1.502657}), facets) == Location::Boundary);
}

