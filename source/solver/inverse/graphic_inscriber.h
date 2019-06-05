// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/location/location.h"
#include "geometry/convex_decomposition/convex_decomposition.h"
#include "grid/rasterization/polytope_rasterizer.h"
#include "grid/sampling/vector_sampling.h"
#include "solver/inverse/minkowski_sum_rasterizer.h"
#include "solver/inverse/minkowski_sum.h"
#include "solver/iterative_inscriber.h"

class GraphicInscriber final : public IterativeInscriber {
public:
    GraphicInscriber(
            ConvexDecompositor convexDecompositor,
            MinkowskiSumRasterizer minkowskiSumRasterizer,
            PolytopeRasterizer contourRasterizer);

private:
    struct GraphicIteration : public Iteration {
        GraphicIteration(
                MinkowskiSum minkowskiSum,
                const Polytope* contour,
                double gridLipschitzConstant,
                VectorSampling<Location> selectionSampling);

        const MinkowskiSum minkowskiSum;
        const Polytope* const contour;
        // Maximum difference in objective value over a voxel
        const double gridLipschitzConstant;

        // All voxels should be outer here,
        // only the selection and mapping is considered
        VectorSampling<Location> selectionSampling;
        double radiusStep;
    };

    virtual std::unique_ptr<Iteration> init(
            const Polytope* pattern,
            const Polytope* contour) const override;
    virtual std::unique_ptr<Iteration> iterate(
        std::unique_ptr<Iteration> previous) const override;

    // selectionSampling is assumed to contain only outer voxels
    VectorSampling<Location> rasterize(
            const MinkowskiSum& minkowskiSum,
            double radius,
            const Polytope& contour,
            const VectorSampling<Location>& selectionSampling) const;

    const ConvexDecompositor convexDecompositor_;
    const MinkowskiSumRasterizer msumRasterizer_;
    const PolytopeRasterizer contourRasterizer_;
};
