// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/convex_decomposition/convex_decomposition.h"
#include "geometry/entity/polytope.h"
#include "geometry/kernel.h"
#include "utility/generator.h"
#include "utility/noncopyable.h"

#include <utility>

class MinkowskiSum final : public NonCopyable {
public:
    struct ConvexPart final : public NonCopyable {
        template<class FacetsSource>
        ConvexPart(
                FacetsSource&& source,
                Facet baseFacet,
                Vector<> innerDirection)
            : facets(std::forward<FacetsSource>(source))
            , baseFacet(std::move(baseFacet))
            , innerDirection(std::move(innerDirection))
        {}

        const Generator<const Facet&> facets;

        // For the determination of the inner region
        const Facet baseFacet;
        // For the baseFacet itself
        const Vector<> innerDirection;
    };

    MinkowskiSum(
            const Polytope& contour,
            const ConvexDecomposition& patternDecomposition);

    // patternScale must be strictly positive
    Generator<const ConvexPart&> convexParts(double patternScale) const;

private:
    struct VertexTemplate {
        Point origin;
        Vector<> direction;
    };
    using FacetTemplate = std::array<VertexTemplate, DIMS>;
    struct ConvexPartTemplate {
        std::vector<FacetTemplate> facets;
        Facet baseFacet;
    };

    static std::vector<ConvexPartTemplate> prepareTemplates(
            const Polytope& contour,
            const ConvexDecomposition& patternDecomposition);
    static ConvexPartTemplate convexSum(
            const Facet& contourFacet,
            const PolytopeConvexPart& patternPart);
    static Vector<> findInnerDirection(const ConvexPartTemplate& partTemplate);

    const std::vector<ConvexPartTemplate> partTemplates_;
};
