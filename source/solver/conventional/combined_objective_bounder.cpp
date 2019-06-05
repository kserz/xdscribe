// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "combined_objective_bounder.h"

#include "grid/rasterization/facet_box_overlap.h"

ValueBounds computeCombinedBounds(
        const Objective& objective,
        const Region& region)
{
    auto result = computeSimpleBounds(objective, region);

    bool outer = result.lower() < MEPS;
    if (outer) {
        // More precise boundaries estimation
        // TODO: Doesn't help much so far,
        //   should check after implementing breaks in generator
        const Point lowerCorner = (region.center() - 0.5 * region.size()).eval();
        objective.contour()->facetGeometries().process(
                [&] (const Facet& facet) {
            if (outer && FacetBoxOverlap(region.size(), facet)(lowerCorner)) {
                // Region is actually boundary
                outer = false;
            }
        });
    }

    if (outer) {
        return ValueBounds{0., 0.};
    } else {
        return result;
    }
}
