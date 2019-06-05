// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "simplex_facet_overlap.h"

#include "geometry/entity/perpendicular.h"
#include "utility/subsets.h"

#include <Eigen/Dense>

#include <algorithm>
#include <limits>

namespace {

struct Interval final {
    double min;
    double max;
};

template<class PointsHull>
Interval projection(const PointsHull& pointsHull, const Vector<>& direction)
{
    Interval result{
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::lowest()
    };
    for (const auto& point : pointsHull) {
        double pointProjection = direction.dot(point);
        result.min = std::min(result.min, pointProjection);
        result.max = std::max(result.max, pointProjection);
    }
    return result;
}

} // namespace

SimplexFacetOverlap::SimplexFacetOverlap(const Simplex* simplex)
    : simplex_(simplex)
{}

// TODO: Speed up by skipping redundant computations
Location SimplexFacetOverlap::operator ()(const Facet& facet) const
{
    // Overlap test by the separating axis theorem
    Location result = Location::Inner;
    for (size_t simplexFaceDim = 0; simplexFaceDim < DIMS; ++simplexFaceDim) {
        subSets<Point>(simplex_, simplexFaceDim + 1).process(
                [&] (auto&& simplexFace) {
            subSets<Point>(&facet, DIMS - simplexFaceDim).process(
                    [&] (auto&& facetFace) {
                // TODO: Think about multiple perpendiculars
                const auto axis = commonPerpendicular(simplexFace, facetFace);
                const auto simplexProjection = projection(*simplex_, axis);
                const auto facetProjection = projection(facet, axis);

                if (simplexProjection.max < facetProjection.min - MEPS ||
                        facetProjection.max < simplexProjection.min - MEPS) {
                    result = Location::Outer;
                } else if (simplexProjection.max < facetProjection.min + MEPS ||
                        facetProjection.max < simplexProjection.min + MEPS) {
                    if (result == Location::Inner) {
                        result = Location::Boundary;
                    }
                }
            });
        });
    }
    return result;
}
