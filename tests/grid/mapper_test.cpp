#include "grid/sampling/mapper.h"

#include <catch2/catch.hpp>

TEST_CASE("mapper")
{
    Mapper mapper(Box({2., 3., 4.}, 2.), 16);

    REQUIRE(mapper.toGlobal(3.) == Approx(3./4.).epsilon(1e-9));
    REQUIRE(mapper.toLocal(3./4.) == Approx(3.).epsilon(1e-9));

    auto test1 = mapper.toLocal({2.5, 6., 0.});
    REQUIRE(test1[0] == Approx(10.).epsilon(1e-9));
    REQUIRE(test1[1] == Approx(20.).epsilon(1e-9));
    REQUIRE(test1[2] == Approx(-8.).epsilon(1e-9));

    auto test2 = mapper.toGlobal(Coordinates({10, 20, -8}));
    REQUIRE(test2[0] == Approx(2.5).epsilon(1e-9));
    REQUIRE(test2[1] == Approx(6.).epsilon(1e-9));
    REQUIRE(test2[2] == Approx(0.).epsilon(1e-9));

    Mapper mapper2(Box({2., 3., 4.}, 2.), 15);

    auto test3 = mapper2.toLocal({2.5, 6., 0.});
    REQUIRE(test3[0] == Approx(9.375).epsilon(1e-9));
    REQUIRE(test3[1] == Approx(18.75).epsilon(1e-9));
    REQUIRE(test3[2] == Approx(-7.5).epsilon(1e-9));
}
