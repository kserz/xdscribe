#include "geometry/location/axis_distance.h"

#include <catch2/catch.hpp>

TEST_CASE("axis distance")
{
    REQUIRE(!AxisDistance(Facet{
        Point{1, 0, 2},
        Point{0, 0, 2},
        Point{0, 1, 2}})
            (Point{4, 5, 1}));

    auto dist0 = AxisDistance(Facet{
        Point{5, 7, 4},
        Point{2, 4, 4},
        Point{9, 6, 4}})
            (Point{4, 5, 1});
    REQUIRE(dist0);
    REQUIRE(dist0.value() == Approx(3.).epsilon(1e-6));

    auto dist1 = AxisDistance(Facet{
        Point{5, 7, 6},
        Point{2, 4, 3},
        Point{9, 6, 5}})
            (Point{4, 5, 2});
    REQUIRE(dist1);
    REQUIRE(dist1.value() == Approx(2.).epsilon(1e-6));

    auto dist2 = AxisDistance(Facet{
        Point{7., 3., 0},
        Point{5, 5, 6.65},
        Point{2, 3, 0}})
            (Point{5, 5, 1});
    REQUIRE(dist2);
    REQUIRE(dist2.value() == Approx(5.65).epsilon(1e-6));

    auto dist3 = AxisDistance(Facet{
        Point{2., 3.5, 0.},
        Point{2.57735, 0.92265, 4.57735},
        Point{2., 1.5, 4.}})
            (Point{2., 1., 0.});
    REQUIRE(!dist3);

    auto dist4 = AxisDistance(Facet{
        Point{1e-12, 3, -4.50797},
        Point{0, 1, -1.50266},
        Point{0, 0, 0.497343}})
            (Point{-1, 1, -6});
    REQUIRE(!dist4);

}
