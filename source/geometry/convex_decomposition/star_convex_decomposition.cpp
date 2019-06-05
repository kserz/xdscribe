// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "star_convex_decomposition.h"

#include <algorithm>

ConvexDecomposition starDecomposition(const Polytope* starShapedPolytope)
{
    return ConvexDecomposition([starShapedPolytope] (auto&& yield) {
        size_t partsCount = 0;
        starShapedPolytope->facetGeometries().process(
                [&] (const Facet& facet) {
            PolytopeConvexPart result(DIMS+1);
            result[0] = Point::constant(0.);
            std::copy(facet.begin(), facet.end(), ++result.begin());
            yield(result);
            ++partsCount;
        });
    });
}
