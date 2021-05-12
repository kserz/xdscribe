#include "geometry/entity/perpendicular.h"
#include "geometry/kernel.h"
#include "utility/generator.h"

#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

#include <numeric>
#include <vector>

TEST_CASE("generator read")
{
    int sum = 0;
    Generator<int> gen([&](auto&& yield)
    {
        for (int i = 1; i < 10; ++i) {
            yield(i);
        }

        REQUIRE(sum == 45);

        for (int i = 1; i < 10; ++i) {
            yield(i);
        }
    });

    gen.process([&](int data)
    {
        sum += data;
    });
    REQUIRE(sum == 90);

    sum = 0;
    gen.process([&](int data)
    {
        sum += data;
    });
    REQUIRE(sum == 90);
}

TEST_CASE("generator write")
{
    std::vector<int> storage(20);
    std::iota(storage.begin(), storage.end(), 0);

    REQUIRE(std::accumulate(storage.begin(), storage.end(), 0) == 190);

    Generator<int&> gen(&storage);
    gen.process([](auto& value) {
        value = 1;
    });
    REQUIRE(std::accumulate(storage.begin(), storage.end(), 0) == 20);
}

TEST_CASE("generator simple benchmark")
{
    std::vector<Coordinates> rawSource(10000);
    for (int i = 0; i < static_cast<int>(rawSource.size()); ++i) {
        rawSource[i] = {i, i + 1, i + 2};
    }
    const auto& source = rawSource;
    // Function based generator does not pass this test
    Generator<const Coordinates&> gen(&source);

    int sourceSum = 0;
    BENCHMARK("loop reference") {
        sourceSum = 0;
        for (const auto& coordinates : source) {
            for (const auto& coordinate : coordinates) {
                sourceSum += coordinate;
            }
        }
    };

    int genSum = 0;
    BENCHMARK("generator") {
        genSum = 0;
        gen.process([&](const auto& coordinates) {
            for (const auto& coordinate : coordinates) {
                genSum += coordinate;
            }
        });
    };

    REQUIRE(genSum == sourceSum);
}

TEST_CASE("generator complex benchmark")
{
    std::vector<Facet> rawSource(1000);
    for (size_t i = 0; i < rawSource.size(); ++i) {
        auto c = static_cast<double>(i);
        rawSource[i] = {
            Point{c, c + 1., c + 2.},
            Point{c + 3., c + 4., c + 5.},
            Point{c + 6., c + 7., c + 8.}
        };
    }
    const auto& source = rawSource;
    Generator<const Facet&> gen([&](auto&& yield) {
        for (const auto& facet : source) {
            yield(facet);
        }
    });

    const auto processFacet = [] (const Facet& facet, double* sum) {
        const Point normal = unitNormal(facet);
        for (auto coordinate : normal) {
            *sum += coordinate;
        }
    };

    double sourceSum = 0.;
    BENCHMARK("loop reference") {
        sourceSum = 0.;
        for (const auto& facet : source) {
            processFacet(facet, &sourceSum);
        }
    };

    double genSum = 0.;
    BENCHMARK("generator"){
        genSum = 0.;
        gen.process([&](const auto& facet) {
            processFacet(facet, &sourceSum);
        });
    };

    REQUIRE(genSum == Approx(sourceSum).epsilon(1e-6));
}
