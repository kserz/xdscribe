#include "geometry/kernel.h"
#include "geometry/entity/perpendicular.h"
#include "utility/lazy.h"

#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

TEST_CASE("lazy usage")
{
    bool initialized = false;

    Lazy<int> cached([&] {
        initialized = true;
        return 42;
    });

    REQUIRE(initialized == false);
    REQUIRE(cached() == 42);
    REQUIRE(initialized == true);
}

TEST_CASE("lazy performance")
{
    const Facet face = {
        Point{1., 2., 3.},
        Point{4., 5., 6.},
        Point{7., 8., 9.}
    };

    const size_t iterations = 1000;
    const Vector<> constNormal = unitNormal(face);
    double constSum = 0.;
    BENCHMARK("reference const") {
        constSum = 0.;
        for (size_t i = 0; i < iterations; ++i) {
            auto c = static_cast<double>(i);
            constSum += constNormal.dot(Vector<>{c, c + 2., c + 4.});
        }
    };

    std::optional<Vector<>> optNormal;
    double optSum = 0.;
    BENCHMARK("reference optional") {
        optSum = 0.;
        for (size_t i = 0; i < iterations; ++i) {
            auto c = static_cast<double>(i);
            if (!optNormal) {
                optNormal = unitNormal(face);
            }
            optSum += optNormal.value().dot(Vector<>{c, c + 2., c + 4.});
        }
    };


    Lazy<Vector<>> lazyNormal([&] {
        return unitNormal(face);
    });
    const auto& normal = lazyNormal();
    double lazySum = 0.;
    BENCHMARK("lazy"){
        lazySum = 0.;
        for (size_t i = 0; i < iterations; ++i) {
            auto c = static_cast<double>(i);
            lazySum += normal.dot(Vector<>{c, c + 2., c + 4.});
        }
    };

    REQUIRE(lazySum == Approx(constSum).epsilon(1e-6));
    REQUIRE(lazySum == Approx(optSum).epsilon(1e-6));
}
