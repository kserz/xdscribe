// This file is part of xscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "minkowski_sum.h"

#include "geometry/entity/perpendicular.h"
#include "geometry/utility/cgal.h"
#include "helper/stats.h"

#include <CGAL/Triangulation.h>

#include <unordered_map>
#include <utility>

namespace {

} // namespace

MinkowskiSum::MinkowskiSum(
        const Polytope& contour,
        const ConvexDecomposition& patternDecomposition)
    : partTemplates_(prepareTemplates(contour, patternDecomposition))
{}

Generator<const MinkowskiSum::ConvexPart&> MinkowskiSum::convexParts(
        double patternScale) const
{
    assert(patternScale > MEPS);
    return Generator<const ConvexPart&>([this, patternScale] (auto&& yield) {
        const auto extractPart = [this, patternScale]
                (const ConvexPartTemplate* partTemplate) {
            return Generator<const Facet&>(
                    [patternScale, partTemplate] (auto&& yield) {
                for (const auto& facetTemplate : partTemplate->facets) {
                    Facet facet;
                    for (size_t i = 0; i < facet.size(); ++i) {
                        const auto& vertexTemplate = facetTemplate[i];
                        facet[i] = (vertexTemplate.origin +
                            patternScale * vertexTemplate.direction).eval();
                    }
                    yield(facet);
                }
            });
        };

        for (const auto& partTemplate : partTemplates_) {
            assert(!partTemplate.facets.empty());
            yield(ConvexPart{
                extractPart(&partTemplate),
                partTemplate.baseFacet,
                findInnerDirection(partTemplate)
            });
        }
    });
}

std::vector<MinkowskiSum::ConvexPartTemplate>
MinkowskiSum::prepareTemplates(
        const Polytope& contour,
        const ConvexDecomposition& patternDecomposition)
{
    std::vector<ConvexPartTemplate> result;
    size_t patternPartsCount = 0;
    patternDecomposition.process([&] (auto&& patternPart) {
        ++patternPartsCount;
        contour.facetGeometries().process([&] (auto&& contourFacet) {
            result.push_back(convexSum(contourFacet, patternPart));
        });
    });
    Stats::instance().patternConvexPartsCount = patternPartsCount;
    return result;
}

MinkowskiSum::ConvexPartTemplate MinkowskiSum::convexSum(
        const Facet& contourFacet,
        const PolytopeConvexPart& patternPart)
{
    using Triangulation = CGAL::Triangulation<Kernel>;
    Triangulation triangulation(DIMS);

    std::unordered_map<Triangulation::Vertex_handle, VertexTemplate>
        vertexTemplates;
    for (const auto& fixedVertex : contourFacet) {
        for (const auto& scalingVertex : patternPart) {
            const auto handle = triangulation.insert(
                toCGAL(Point(fixedVertex + scalingVertex)));
            vertexTemplates[handle] = {fixedVertex, scalingVertex};
        }
    }

    assert(triangulation.current_dimension() == DIMS);

    using FullCells = std::vector<Triangulation::Full_cell_handle>;
    FullCells infiniteFullCells;
    auto out = std::back_insert_iterator<FullCells>(infiniteFullCells);
    triangulation.incident_full_cells(triangulation.infinite_vertex(), out);

    std::vector<FacetTemplate> facets;
    facets.reserve(infiniteFullCells.size());
    for (const auto& fullCell : infiniteFullCells) {
        FacetTemplate resultFacet;
        for (size_t i = 0; i < DIMS + 1; ++i) {
            const size_t vertexIndex =
                static_cast<int>(i) <=
                    fullCell->index(triangulation.infinite_vertex()) ?
                    i : i - 1;
            if (vertexIndex == DIMS) {
                continue;
            }
            resultFacet[vertexIndex] = vertexTemplates[fullCell->vertex(i)];
        }
        facets.push_back(std::move(resultFacet));
    }

    return ConvexPartTemplate{
        std::move(facets),
        contourFacet
    };
}

Vector<> MinkowskiSum::findInnerDirection(
        const MinkowskiSum::ConvexPartTemplate& partTemplate)
{
    auto baseNormal = unitNormal(partTemplate.baseFacet);
    for (const auto& facetTemplate : partTemplate.facets) {
        for (const auto& vertexTemplate : facetTemplate) {
            if (std::fabs(vertexTemplate.direction.dot(baseNormal)) > MEPS) {
                return vertexTemplate.direction;
            }
        }
    }
    // Non-degenerate patterns never reach here
    assert(false);
    return partTemplate.facets[0][0].direction;
}
