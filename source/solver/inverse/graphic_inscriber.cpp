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
        DomainEstimatorFactory domainEstimatorFactory,
        AccuracyEstimatorFactory accuracyEstimatorFactory)
    : convexDecompositor_(std::move(convexDecompositor))
    , domainEstimatorFactory_(std::move(domainEstimatorFactory))
    , accuracyEstimatorFactory_(std::move(accuracyEstimatorFactory))
{}

GraphicInscriber::GraphicIteration::GraphicIteration(
        std::unique_ptr<MinkowskiSum> minkowskiSum,
        std::unique_ptr<DomainEstimator> domainEstimator,
        std::unique_ptr<ActualAccuracyEstimator> actualAccuracyEstimator,
        double gridLipschitzConstant,
        VectorSampling<Location> sampling)
    : Iteration(
          sampling.container().radius(),
          Placement{sampling.container().center(), 0.})
    , minkowskiSum_(std::move(minkowskiSum))
    , domainEstimator_(std::move(domainEstimator))
    , actualAccuracyEstimator_(std::move(actualAccuracyEstimator))
    , gridLipschitzConstant(gridLipschitzConstant)
    , sampling(std::move(sampling))
    , radiusStep(this->sampling.gridStep() * RADIUS_STEP_RATIO)
{}

std::unique_ptr<IterativeInscriber::Iteration>
GraphicInscriber::init(
        const Polytope* pattern,
        const Polytope* contour,
        double targetPrecision) const
{
    const auto invertedPattern = Polytope::invert(*pattern);
    auto minkowskiSum = std::make_unique<MinkowskiSum>(
        *contour,
        convexDecompositor_(&invertedPattern));
    auto domainEstimator = std::make_unique<DomainEstimator>(
        domainEstimatorFactory_(minkowskiSum.get(), contour));
    auto actualAccuracyEstimator = std::make_unique<ActualAccuracyEstimator>(
        accuracyEstimatorFactory_(
            domainEstimator.get(),
            pattern,
            targetPrecision));
    // Initially all the voxels are in undefined state
    const auto sampling = VectorSampling<Location>{
        boundingBox(contour->vertices()),
        8,
        Location::Boundary};

    return std::make_unique<GraphicIteration>(
        std::move(minkowskiSum),
        std::move(domainEstimator),
        std::move(actualAccuracyEstimator),
        InscribedRadius::lipschitzConstant(*pattern) * std::sqrt(DIMS),
        std::move(sampling));
}

std::unique_ptr<IterativeInscriber::Iteration> GraphicInscriber::iterate(
        std::unique_ptr<IterativeInscriber::Iteration> iteration) const
{
    auto& it = static_cast<GraphicIteration&>(*iteration);

    const auto newSampling = (*it.domainEstimator_)(
        it.sampling,
        it.solution.radius() + it.radiusStep);

    reportSamplingDistribution(newSampling);

    bool haveFilledVoxel = false;
    Coordinates lastFilledCoords = Coordinates::constant(0);
    newSampling.voxels().process([&] (const auto& voxel) {
        if (voxel.value == Location::Inner) {
            haveFilledVoxel = true;
            lastFilledCoords = voxel.coordinates();
        }
    });

    if (haveFilledVoxel) {
        it.solution = {
            it.sampling.toGlobal(lastFilledCoords),
            it.solution.radius() + it.radiusStep
        };

        it.sampling = shrink(newSampling);
    } else {
        it.precision = (*it.actualAccuracyEstimator_)(
            it.solution.radius(),
            it.radiusStep,
            it.sampling);

        it.sampling = refine(it.sampling);

        it.radiusStep /= 2.;
    }

    return std::move(iteration);
}
