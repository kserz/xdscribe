// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "graphic_inscriber.h"

#include "geometry/entity/bounding_box.h"
#include "geometry/location/location.h"
#include "grid/sampling/refinement.h"
#include "solver/inscribed_radius.h"

const double RADIUS_STEP_RATIO = 1.;

GraphicInscriber::GraphicInscriber(
        ConvexDecompositor convexDecompositor,
        MinkowskiSumRasterizer minkowskiSumRasterizer,
        PolytopeRasterizer contourRasterizer)
    : convexDecompositor_(std::move(convexDecompositor))
    , msumRasterizer_(std::move(minkowskiSumRasterizer))
    , contourRasterizer_(std::move(contourRasterizer))
{}

GraphicInscriber::GraphicIteration::GraphicIteration(
        MinkowskiSum minkowskiSum,
        const Polytope* contour,
        double patternStarRatio,
        VectorSampling<Location> sampling)
    : Iteration(
          sampling.container().radius(),
          Placement{sampling.container().center(), 0.})
    , minkowskiSum(std::move(minkowskiSum))
    , contour(contour)
    , gridLipschitzConstant(patternStarRatio)
    , selectionSampling(std::move(sampling))
    , radiusStep(this->selectionSampling.gridStep() * RADIUS_STEP_RATIO)
{}

std::unique_ptr<IterativeInscriber::Iteration>
GraphicInscriber::init(const Polytope* pattern, const Polytope* contour) const
{
    const auto invertedPattern = Polytope::invert(*pattern);
    auto minkowskiSum = MinkowskiSum{
            *contour,
            convexDecompositor_(&invertedPattern)};
    const auto sampling = VectorSampling<Location>{
        boundingBox(contour->vertices()),
        8,
        Location::Outer};

    return std::make_unique<GraphicIteration>(
        std::move(minkowskiSum),
        contour,
        InscribedRadius::lipschitzConstant(*pattern) * std::sqrt(DIMS),
        std::move(sampling));
}

std::unique_ptr<IterativeInscriber::Iteration> GraphicInscriber::iterate(
        std::unique_ptr<IterativeInscriber::Iteration> iteration) const
{
    auto& it = static_cast<GraphicIteration&>(*iteration);

    auto newSampling = rasterize(
        it.minkowskiSum,
        it.solution.radius() + it.radiusStep,
        *it.contour,
        it.selectionSampling);

    bool haveEmptyVoxel = false;
    Coordinates lastEmptyCoords = Coordinates::constant(0);
    newSampling.voxels().process([&] (const auto& voxel) {
        if (voxel.value == Location::Outer) {
            haveEmptyVoxel = true;
            lastEmptyCoords = voxel.coordinates();
        }
    });

    if (haveEmptyVoxel) {
        it.solution = {
            it.selectionSampling.toGlobal(lastEmptyCoords),
            it.solution.radius() + it.radiusStep
        };

        it.selectionSampling = shrink(newSampling);
    } else {
        it.precision = it.radiusStep +
            it.gridLipschitzConstant *
            it.selectionSampling.gridStep();

        it.selectionSampling = refine(it.selectionSampling);

        it.radiusStep /= 2.;
    }

    return std::move(iteration);
}

VectorSampling<Location> GraphicInscriber::rasterize(
        const MinkowskiSum& minkowskiSum,
        double radius,
        const Polytope& contour,
        const VectorSampling<Location>& selectionSampling) const
{
    auto result = selectionSampling;
    msumRasterizer_(minkowskiSum, radius, &result);

    VectorSparseRaster<Location> feasibility = selectionSampling;
    assert(contourRasterizer_);
    contourRasterizer_(
        selectionSampling.toLocal(contour.facetGeometries()),
        &feasibility);

    result.voxels().process([&] (auto& voxel) {
        auto* feasibilityVoxel = feasibility.find(voxel.coordinates());
        assert(feasibilityVoxel);
        if (feasibilityVoxel->value == Location::Outer) {
            voxel.value = Location::Inner;
        }
    });

    return result;
}
