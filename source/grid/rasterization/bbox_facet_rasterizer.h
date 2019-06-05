// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "grid/rasterization/facet_rasterizer.h"

// Facet rasterizer with additional bounding box tests
// For facets spanning less than coarseThreshold along any coordinate axis
// the whole bounding box is rasterized omitting the expensive overlap test
// coarseThreshold = 0 leads to precise facets rasterization
// Good coarseThreshold values are somewhere between 1 and 3.
FacetRasterizer bBoxFacetRasterizer(double coarseThreshold = 2.);
