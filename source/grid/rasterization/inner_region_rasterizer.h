// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/location/location.h"
#include "grid/sampling/sparse_raster.h"

#include <functional>

// Draws the polytope inner region
// assuming the boundary to be drawn already.
// Only changes the voxels covered by the inner region
using InnerRegionRasterizer = std::function<void(
    const Generator<const Facet&>& localPolytopeGeometry,
    SparseRaster<Location>* raster)>;

// Every facent and every node is processed independently
void rasterizeInnerRegionSequentally(
        const Generator<const Facet&>& localPolytopeGeometry,
        SparseRaster<Location>* raster);

// Geometry is processed only once but
// all the node computations are independent
void rasterizeInnerRegionByFacets(
        const Generator<const Facet&>& localPolytopeGeometry,
        SparseRaster<Location>* raster);

// Geometry is processed only once and
// the node computations are made simultaneously
// for the whole line along the last axis
void rasterizeInnerRegionByRays(
        const Generator<const Facet&>& localPolytopeGeometry,
        SparseRaster<Location>* raster);
