#include "geometry/location/simplex_facet_overlap.h"

#include <catch2/catch.hpp>

TEST_CASE("overlap simplex facet")
{
    Simplex simplex{
        Point{0., 0., 0.},
        Point{1., 0., -0.562018},
        Point{0., 1., -0.562018},
        Point{-0.5, 0.5, -0.314237}
    };
    Facet facet{
        Point{1., 0., -0.562018},
        Point{0., 1., -0.562018},
        Point{-0.5, 0.5, -0.314237}
    };
    REQUIRE(SimplexFacetOverlap(&simplex)(facet) == Location::Boundary);
}
