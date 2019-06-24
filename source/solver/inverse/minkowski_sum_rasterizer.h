// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/location/location.h"
#include "solver/inverse/minkowski_sum.h"
#include "grid/rasterization/polytope_rasterizer.h"
#include "grid/sampling/sampling.h"
#include "grid/sampling/sparse_raster.h"
#include "utility/generator.h"

#include <functional>

// Only changes the voxels covered by the image
using MinkowskiSumRasterizer = std::function<void(
    const MinkowskiSum& minkowskiSum,
    double patternScale,
    Sampling<Location>* sampling)>;

// Assumes partSampling to be empty (all voxels outer)
using ConvexPartRasterizer = std::function<void(
    const MinkowskiSum::ConvexPart& convexPart,
    Sampling<Location>* partSampling)>;

void combineImages(
        const SparseRaster<Location>& partImage,
        SparseRaster<Location>* combinedImage);

template<template<class> class PartSampling>
MinkowskiSumRasterizer decomposingMSRasterizer(
        ConvexPartRasterizer convexPartRasterizer)
{
    return [convexPartRasterizer = std::move(convexPartRasterizer)] (
            const MinkowskiSum& minkowskiSum,
            double patternScale,
            Sampling<Location>* sampling)
    {
        minkowskiSum.convexParts(patternScale).process(
            [&] (const MinkowskiSum::ConvexPart& convexPart) {
                PartSampling<Location> partSampling{
                    *sampling,
                    typename PartSampling<Location>::Raster{
                        compositeGenerator<const Coordinates&>(
                            sampling->voxels(),
                            [] (const auto& voxel, auto&& yield) {
                                if (voxel.value != Location::Inner) {
                                    yield(voxel.coordinates());
                                }
                            }),
                        Location::Outer,
                        sampling->size()
                    }
                };

                convexPartRasterizer(convexPart, &partSampling);
                combineImages(partSampling, sampling);
            });
    };
}

ConvexPartRasterizer polytopePartRasterizer(
        PolytopeRasterizer polytopeRasterizer);
