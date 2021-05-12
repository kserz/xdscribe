#include "geometry/entity/bounding_box.h"
#include "geometry/entity/placement.h"
#include "geometry/entity/polytope.h"
#include "tests/geometry/helpers.h"

#include <catch2/catch.hpp>

#include <cmath>
#include <fstream>
#include <vector>

TEST_CASE("polytope load")
{
    auto p = Polytope::loadObj("examples/tetrahedron_4.obj");

    std::vector<Facet> facets;
    p.facetGeometries().process([&](const Facet& face)
    {
        facets.push_back(face);
    });

    REQUIRE(facets.size() == 4);

    Facet facet2 = {
        Point{0.866, -0.5, -1.503},
        Point{0., 0., 0.497},
        Point{-0.866, -0.5, -1.503}
    };
    REQUIRE(facetsEqual(facets[2], facet2));
}

TEST_CASE("polytope bbox")
{
    auto p = Polytope::loadObj("examples/tetrahedron_4.obj");
    const Box bBox = boundingBox(p.vertices());
    REQUIRE(bBox.radius() == Approx(1.).epsilon(1e-2));
    Point center({0., 0.25, -0.5});
    for (size_t i = 0; i < DIMS; ++i) {
        REQUIRE(center[i] == Approx(bBox.center()[i]).epsilon(1e-2));
    }
}

TEST_CASE("polytope neighbor facets")
{
    auto p = Polytope::loadObj("examples/tetrahedron_4.obj");
    for (size_t facetIndex = 0; facetIndex < p.facetTopologies().size(); ++facetIndex) {
        size_t count = 0;
        p.neighborFacetIndices(facetIndex).process([&] (auto neighborIndex) {
            REQUIRE(neighborIndex != facetIndex);
            REQUIRE(neighborIndex < p.facetTopologies().size());
            ++count;
        });
        REQUIRE(count == DIMS);
    }
}
