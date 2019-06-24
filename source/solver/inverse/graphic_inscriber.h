// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/convex_decomposition/convex_decomposition.h"
#include "geometry/location/location.h"
#include "grid/sampling/vector_sampling.h"
#include "solver/inverse/accuracy_estimator.h"
#include "solver/inverse/domain_estimator.h"
#include "solver/inverse/minkowski_sum.h"
#include "solver/iterative_inscriber.h"

class GraphicInscriber final : public IterativeInscriber {
public:
    GraphicInscriber(
            ConvexDecompositor convexDecompositor,
            DomainEstimatorFactory domainEstimatorFactory,
            AccuracyEstimatorFactory accuracyEstimatorFactory);

private:
    struct GraphicIteration : public Iteration {
        GraphicIteration(
                std::unique_ptr<MinkowskiSum> minkowskiSum,
                std::unique_ptr<DomainEstimator> domainEstimator,
                std::unique_ptr<ActualAccuracyEstimator> actualAccuracyEstimator,
                double gridLipschitzConstant,
                VectorSampling<Location> sampling);

        const std::unique_ptr<MinkowskiSum> minkowskiSum_;
        const std::unique_ptr<DomainEstimator> domainEstimator_;
        const std::unique_ptr<ActualAccuracyEstimator> actualAccuracyEstimator_;
        // Maximum difference in objective value over a voxel
        const double gridLipschitzConstant;

        VectorSampling<Location> sampling;
        double radiusStep;
    };

    virtual std::unique_ptr<Iteration> init(
            const Polytope* pattern,
            const Polytope* contour,
            double targetPrecision) const override;
    virtual std::unique_ptr<Iteration> iterate(
        std::unique_ptr<Iteration> previous) const override;

    const ConvexDecompositor convexDecompositor_;
    const DomainEstimatorFactory domainEstimatorFactory_;
    const AccuracyEstimatorFactory accuracyEstimatorFactory_;
};
