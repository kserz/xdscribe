// This file is part of xscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "facet_box_overlap.h"

#include "geometry/entity/perpendicular.h"

#include <Eigen/Dense>

namespace {

// Return a normal to the facet edge in the plane perpendicular to an axis.
// Result is oriented outside the facet.
// Edge is specified by the starting point index.
Vector<> evalEdgeNormal(
        unsigned axis,
        const Facet& facet,
        unsigned edge)
{
    assert(DIMS == 3);
    const std::array<Point, 2> edgePoints{
        facet[edge],
        facet[edge + 1 < DIMS ? edge + 1 : 0]
    };
    const std::array<Point, 2> axisPoints {
        Point::constant(0.),
        Vector<>::Unit(axis).eval()
    };
    auto result = commonPerpendicular(edgePoints, axisPoints);

    auto oppositeVertexIndex = edge == 0 ? DIMS - 1 : edge - 1;
    if (result.dot(facet[oppositeVertexIndex] - facet[edge]) > 0.) {
        result *= -1.;
    }

    return result;
}

} // namespace

Point FacetBoxOverlap::lowestPoint(
        const Vector<>& boxSize,
        const Vector<>& normal)
{
    auto result = Point::constant(0.);
    for (size_t i = 0; i < DIMS; ++i) {
        if (normal[i] < -MEPS) {
            result[i] = boxSize[i];
        }
    }
    return result;
}

FacetBoxOverlap::FacetBoxOverlap(Vector<> boxSize, Facet facet)
    : boxSize_(std::move(boxSize))
    , facet_(std::move(facet))
    , facetBoundingBox_(boundingBox(facet_))
    , lazyFacetNormal_([this] {
        return unitNormal(facet_);
    })
    , lazyPlaneTests_([this] {
        return buildPlaneTests(facet_, lazyFacetNormal_, boxSize_);
    })
    , lazyAxisDegenerate_([this] {
        return buildAxisDegenerate(lazyFacetNormal_);
    })
    , lazyEdgeTests_([this] {
        return buildEdgeTests(facet_, boxSize_);
    })
{}

bool FacetBoxOverlap::operator ()(const Point& lowerCorner) const
{
    for (size_t i = 0; i < DIMS; ++i) {
        if (lowerCorner[i] > facetBoundingBox_.max()[i] ||
                lowerCorner[i] + boxSize_[i] < facetBoundingBox_.min()[i]) {
            return false;
        }
    }

    const auto& planeTests = lazyPlaneTests_();
    if (planeTests[0](lowerCorner) *
            planeTests[1](lowerCorner) > 0.) {
        return false;
    }

    const auto& axisDegenerate = lazyAxisDegenerate_();
    const auto& edgeTests = lazyEdgeTests_();
    for (unsigned axis = 0; axis < DIMS; ++axis) {
        if (axisDegenerate[axis]) {
            continue;
        }
        for (unsigned edge = 0; edge < DIMS; ++edge) {
            if (edgeTests[axis][edge](lowerCorner) > MEPS) {
                return false;
            }
        }
    }

    return true;
}

FacetBoxOverlap::PlaneTests FacetBoxOverlap::buildPlaneTests(
        const Facet& facet,
        const Lazy<Vector<>>& lazyFacetNormal,
        const Vector<>& boxSize)
{
    const auto& facetNormal = lazyFacetNormal();
    auto planeCriticalPoint = lowestPoint(boxSize, facetNormal);
    return {
        LinearTest{
            facetNormal,
            facetNormal.dot(planeCriticalPoint - facet[0])
        },
        LinearTest{
            facetNormal,
            facetNormal.dot(
                boxSize - planeCriticalPoint - facet[0])
        }
    };
}

FacetBoxOverlap::AxisDegenerate FacetBoxOverlap::buildAxisDegenerate(
        const Lazy<Vector<>>& lazyFacetNormal)
{
    AxisDegenerate result;
    const auto& facetNormal = lazyFacetNormal();
    for (size_t i = 0; i < DIMS; ++i) {
        result[i] = std::fabs(facetNormal[i]) < MEPS;
    }
    return result;
}

FacetBoxOverlap::EdgeTests FacetBoxOverlap::buildEdgeTests(
        const Facet& facet,
        const Vector<>& boxSize)
{
    EdgeTests result;
    // Edges are DIMS-2 faces indexed by starting point in the facet sequence
    // TODO: Think about higher dimensions
    if (DIMS < 3) {
        return result;
    }
    assert(DIMS == 3);
    for (size_t axis = 0; axis < DIMS; ++axis) {
        for (unsigned edge = 0; edge < DIMS; ++edge) {
            auto normal = evalEdgeNormal(axis, facet, edge);
            auto critPoint = lowestPoint(boxSize, normal);
            result[axis][edge] = {
                normal,
                normal.dot(critPoint - facet[edge])
            };
        }
    }
    return result;
}
