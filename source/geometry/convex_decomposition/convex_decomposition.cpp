// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "convex_decomposition.h"

ConvexDecomposition dummyDecomposition(const Polytope* convexPolytope)
{
    return ConvexDecomposition([convexPolytope] (auto&& yield) {
        yield(convexPolytope->vertices());
    });
}
