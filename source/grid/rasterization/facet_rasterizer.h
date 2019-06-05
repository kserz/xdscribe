// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/kernel.h"
#include "geometry/location/location.h"
#include "grid/sampling/sparse_raster.h"

#include <functional>

// Only changes the voxels covered by the image
using FacetRasterizer = std::function<void(
    const Facet& localFacet,
    SparseRaster<Location>* raster)>;

void rasterizeFacetByOverlap(
        const Facet& localFacet,
        SparseRaster<Location>* raster);
