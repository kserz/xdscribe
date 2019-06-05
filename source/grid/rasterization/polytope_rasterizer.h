// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/kernel.h"
#include "geometry/location/location.h"
#include "grid/rasterization/facet_rasterizer.h"
#include "grid/rasterization/inner_region_rasterizer.h"
#include "grid/sampling/sparse_raster.h"
#include "utility/generator.h"

#include <functional>

// Only changes the voxels covered by the image
using PolytopeRasterizer = std::function<void(
    const Generator<const Facet&>& localPolytopeGeometry,
    SparseRaster<Location>* raster)>;

PolytopeRasterizer polytopeRasterizer(
        FacetRasterizer facetRasterizer,
        InnerRegionRasterizer innerRegionRasterizer);
