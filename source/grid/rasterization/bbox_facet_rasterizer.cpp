// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "bbox_facet_rasterizer.h"

#include "geometry/entity/bounding_box.h"
#include "grid/rasterization/facet_box_overlap.h"
#include "grid/sampling/xd_iterator.h"

FacetRasterizer bBoxFacetRasterizer(double coarseThreshold)
{
    return [coarseThreshold] (
            const Facet& localFacet, SparseRaster<Location>* raster)
    {
        const auto facetBBox = boundingBox(localFacet);
        const auto min = intFloor(facetBBox.min());
        const auto max = intFloor(facetBBox.max());
        const auto size = (max - min + Coordinates::constant(1)).eval();

        // Small facet => coarse rasterization
        if ((facetBBox.max() - facetBBox.min()).maxCoeff() < coarseThreshold) {
            XDIterator<DIMS>::run(size, [&] (const Coordinates& offset) {
                auto* it = raster->find((min + offset).eval());
                if (it) {
                    it->value = Location::Boundary;
                }
            });
            return;
        }

        // Precise overlap rasterization
        FacetBoxOverlap voxelOverlap(
            Vector<>::Constant(1.),
            localFacet);

        raster->voxels().process([&] (Voxel<Location>& voxel) {
            for (size_t i = 0; i < DIMS; ++i) {
                if (voxel.coordinates()[i] < min[i] ||
                        voxel.coordinates()[i] > max[i]) {
                    return;
                }
            }

            if (voxelOverlap(voxel.coordinates().cast<double>().eval())) {
                voxel.value = Location::Boundary;
            }
        });
    };
}
