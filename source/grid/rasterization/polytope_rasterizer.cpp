// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "polytope_rasterizer.h"

PolytopeRasterizer polytopeRasterizer(
        FacetRasterizer facetRasterizer,
        InnerRegionRasterizer innerRegionRasterizer)
{
    return [
        facetRasterizer = std::move(facetRasterizer),
        innerRegionRasterizer = std::move(innerRegionRasterizer)] (
                const Generator<const Facet&>& localPolytopeGeometry,
                SparseRaster<Location>* raster)
        {
            localPolytopeGeometry.process([&] (const Facet& facet) {
                facetRasterizer(facet, raster);
            });

            innerRegionRasterizer(localPolytopeGeometry, raster);
        };
}
