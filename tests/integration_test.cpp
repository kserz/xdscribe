#include "geometry/convex_decomposition/floodfill_convex_decomposition.h"
#include "grid/rasterization/bbox_facet_rasterizer.h"
#include "grid/sampling/vector_sparse_raster.h"
#include "solver/conventional/combined_objective_bounder.h"
#include "solver/conventional/ghj_inscriber.h"
#include "solver/inscriber.h"
#include "solver/inverse/graphic_inscriber.h"

#include <catch2/catch.hpp>

#include <iostream>

void check(
        const std::string& patternFile,
        const std::string& contourFile,
        double targetValue,
        double precision = 1e-3)
{
    std::cerr << "Testing " << contourFile << ", " << patternFile << std::endl;

    auto pattern = Polytope::loadObj(std::string("examples/") + patternFile + ".obj");
    auto contour = Polytope::loadObj(std::string("examples/") + contourFile + ".obj");

    auto polytopeRasterizer_ = polytopeRasterizer(
                    bBoxFacetRasterizer(),
                    rasterizeInnerRegionByRays);
    auto graphic_inscriber = GraphicInscriber(
                floodFillDecomposition,
                graphicDomainEstimatorFactory(
                    decomposingMSRasterizer<VectorSampling>(
                        polytopePartRasterizer(polytopeRasterizer_)),
                    polytopeRasterizer_),
                lipschitzianAccuracyEstimatorFactory());
    auto result = graphic_inscriber(
        pattern,
        contour,
        Inscriber::StopPredicate(precision, std::nullopt, 10s));

    auto ghj_inscriber = GHJInscriber(computeCombinedBounds);
    auto reference_result = ghj_inscriber(
        pattern,
        contour,
        Inscriber::StopPredicate(precision, std::nullopt, 30s));

    REQUIRE(result.radius() ==
        Approx(reference_result.radius()).margin(precision));
    REQUIRE(result.radius() == Approx(targetValue).margin(precision));
}

TEST_CASE("integration small data")
{
    check("box_12", "box_12", 1.);
    check("tetrahedron_4", "tetrahedron_4", 1.);

    check("box_12", "tetrahedron_4", 0.57339);
    check("tetra_144", "tetrahedron_4", 1.4504);
    check("heart_320", "tetrahedron_4", 0.49496);

    check("tetrahedron_4", "tetra_144", 0.46161);
}

TEST_CASE("integration medium data")
{
    check("box_12", "heart_320", 0.61984, 1e-2);
    check("box_12", "box_108", 0.80729, 1e-2);
    check("box_12", "tetra_144", 0.34784, 1e-2);

    check("tetrahedron_4", "heart_320", 0.40368, 1e-1);
}
