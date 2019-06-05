// This file is part of xscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "facet_rasterizer.h"

#include "grid/rasterization/facet_box_overlap.h"

void rasterizeFacetByOverlap(
        const Facet& localFacet,
        SparseRaster<Location>* raster)
{
        FacetBoxOverlap voxelOverlap(
            Vector<>::Constant(1.),
            localFacet);

        raster->voxels().process([&] (Voxel<Location>& voxel) {
            if (voxelOverlap(voxel.coordinates().cast<double>().eval())) {
                voxel.value = Location::Boundary;
            }
        });
}
