// This file is part of xscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "floodfill_convex_decomposition.h"

#include "geometry/convex_decomposition/convex_part_builder.h"
#include "geometry/entity/polytope.h"
#include "geometry/kernel.h"
#include "geometry/location/location.h"

#include <queue>
#include <vector>

namespace {

// filler expected to return true if the argument was actually filled
//     and its neighbors should be explored further
// seed is assumed to be filled already
template<class Element, class NeighborsGenerator, class Filler>
void floodFillBFS(
        Element seed,
        NeighborsGenerator&& neighborsGenerator,
        Filler&& fill)
{
    std::queue<Element> searchFront;
    searchFront.push(std::move(seed));

    while (!searchFront.empty()) {
        const auto element = searchFront.front();
        searchFront.pop();

        neighborsGenerator(std::move(element)).process([&] (auto&& neighbor) {
            if (fill(neighbor)) {
                searchFront.push(neighbor);
            }
        });
    }
}

} // namespace

ConvexDecomposition floodFillDecomposition(const Polytope* starShapedPolytope)
{
    assert(locatePoint(
        Point::constant(0),
        starShapedPolytope->facetGeometries()) == Location::Inner);

    return ConvexDecomposition([starShapedPolytope] (auto&& yield) {
        std::vector<bool> usedFacets(
            starShapedPolytope->facetTopologies().size(), false);

        auto nextUnusedFacetIndex = [&] (auto&& facetIndex) {
            for (;facetIndex < usedFacets.size() &&
                    usedFacets[facetIndex]; ++facetIndex);
            return facetIndex;
        };

        for (Polytope::FacetIndex seedFacetIndex = 0;
                seedFacetIndex < usedFacets.size();
                seedFacetIndex = nextUnusedFacetIndex(seedFacetIndex)) {
            ConvexPartBuilder convexPartBuilder(
                starShapedPolytope, seedFacetIndex);
            usedFacets[seedFacetIndex] = true;

            floodFillBFS(
                seedFacetIndex,
                [&] (auto&& facetIndex) {
                    return starShapedPolytope->neighborFacetIndices(facetIndex);
                },
                [&] (auto&& newFacetIndex) {
                    if (!usedFacets[newFacetIndex] &&
                            convexPartBuilder.tryAddFacet(newFacetIndex)) {
                        usedFacets[newFacetIndex] = true;
                        return true;
                    } else {
                        return false;
                    }
                });

            yield(convexPartBuilder.vertices());
        }
    });
}
