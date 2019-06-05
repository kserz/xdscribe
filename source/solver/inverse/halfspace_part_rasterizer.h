// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/location/location.h"
#include "grid/sampling/sampling.h"
#include "solver/inverse/minkowski_sum.h"

// Rasterize convex part as an intersection of half-spaces
void rasterizePartByHalfspaces(
        const MinkowskiSum::ConvexPart& convexPart,
        Sampling<Location>* partSampling);
