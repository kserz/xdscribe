#pragma once

#include "geometry/entity/polytope.h"
#include "geometry/location/location.h"
#include "grid/sampling/sampling.h"
#include "solver/inverse/domain_estimator.h"

#include <functional>

// Returns a proven upper bound on difference
// between radius and global optimal solution
// within inverse graphic solver
using ActualAccuracyEstimator = std::function<double(
    double radius,
    double radiusAccuracy,
    const Sampling<Location>& sampling)>;

using AccuracyEstimatorFactory = std::function<ActualAccuracyEstimator(
    const DomainEstimator* domainEstimator,
    const Polytope* starShapedPattern,
    double targetPrecision)>;

AccuracyEstimatorFactory lipschitzianAccuracyEstimatorFactory();
