// This file is part of xscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/kernel.h"
#include "geometry/entity/polytope.h"
#include "utility/generator.h"

#include <functional>
#include <vector>

using PolytopeConvexPart = std::vector<Point>;
using ConvexDecomposition = Generator<const PolytopeConvexPart&>;
using ConvexDecompositor = std::function<ConvexDecomposition(const Polytope*)>;

// No convexity check is performed!
ConvexDecomposition dummyDecomposition(const Polytope* convexPolytope);
