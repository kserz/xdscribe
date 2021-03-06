// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/convex_decomposition/convex_decomposition.h"

// No star-shapeness check is performed!
ConvexDecomposition floodFillDecomposition(const Polytope* starShapedPolytope);
