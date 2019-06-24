#pragma once

#include "geometry/entity/polytope.h"
#include "geometry/location/location.h"
#include "grid/rasterization/polytope_rasterizer.h"
#include "grid/sampling/sampling.h"
#include "grid/sampling/vector_sampling.h"
#include "solver/inverse/minkowski_sum.h"
#include "solver/inverse/minkowski_sum_rasterizer.h"

#include <functional>

// Returns sampling with the following meaning:
//   Outer = Empty (have no solutions of given radius inside)
//   Boundary = Boundary,
//   Inner = Filled (consists completeley of solutions of at least given radius)
using DomainEstimator = std::function<VectorSampling<Location>(
    // No values in sampling are considered, only selection
    const Sampling<Location>& sampling,
    double radius)>;

using DomainEstimatorFactory = std::function<DomainEstimator(
    const MinkowskiSum* minkowskiSum,
    const Polytope* contour)>;

DomainEstimatorFactory graphicDomainEstimatorFactory(
        MinkowskiSumRasterizer minkowskiSumRasterizer,
        PolytopeRasterizer contourRasterizer);
