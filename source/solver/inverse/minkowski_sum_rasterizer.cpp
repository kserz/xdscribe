// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "minkowski_sum_rasterizer.h"

void combineImages(
        const SparseRaster<Location>& partImage,
        SparseRaster<Location>* combinedImage)
{
    partImage.voxels().process([&] (const auto& voxel) {
        auto* unitedVoxel = combinedImage->find(voxel.coordinates());
        assert(unitedVoxel);
        auto& unitedValue = unitedVoxel->value;

        // Skip already inner voxels
        if (unitedValue == Location::Outer) {
            unitedValue = voxel.value;
        } else if (unitedValue == Location::Boundary &&
                   voxel.value == Location::Inner) {
            unitedValue = Location::Inner;
        }
    });
}

ConvexPartRasterizer polytopePartRasterizer(
        PolytopeRasterizer polytopeRasterizer)
{
    return [polytopeRasterizer = std::move(polytopeRasterizer)] (
            const MinkowskiSum::ConvexPart& convexPart,
            Sampling<Location>* partSampling) {
        assert(polytopeRasterizer);
        polytopeRasterizer(
            partSampling->toLocal(convexPart.facets),
            partSampling);
    };
}
