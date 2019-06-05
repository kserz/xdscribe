// This file is part of xscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/entity/bounding_box.h"
#include "geometry/kernel.h"
#include "geometry/location/linear_test.h"
#include "utility/lazy.h"
#include "utility/noncopyable.h"

// Overlap test for a facet and multiple boxes of the same size
// with lazy initialization.
// See Fast parallel surface and solid voxelization on GPUs
// by Schwarz, Seidel, 2010
class FacetBoxOverlap final : public NonCopyable {
public:
    // Lowest point of an axis-aligned box along the normal direction
    static Point lowestPoint(const Vector<>& boxSize, const Vector<>& normal);

    FacetBoxOverlap(Vector<> boxSize, Facet facet);

    bool operator ()(const Point& lowerCorner) const;

private:
    // Test location relative to facet plane
    // for the box critical point and the opposite one.
    using PlaneTests = std::array<LinearTest, 2>;
    // Axes havig singular facet projection
    using AxisDegenerate = std::array<bool, DIMS>;
    // Test of the critical point location relative to every edge projection
    // along the fixed coordinate axis.
    // By edge we denote any (DIMS-2)-face, in 2d they are just empty.
    using EdgeProjectionsTest = std::array<LinearTest, DIMS>;
    // axis -> edge projection tests
    using EdgeTests = std::array<EdgeProjectionsTest, DIMS>;

    static PlaneTests buildPlaneTests(
            const Facet& facet,
            const Lazy<Vector<>>& lazyFacetNormal,
            const Vector<>& boxSize);
    static AxisDegenerate buildAxisDegenerate(
            const Lazy<Vector<>>& lazyFacetNormal);
    static EdgeTests buildEdgeTests(
            const Facet& facet,
            const Vector<>& boxSize);

    const Vector<> boxSize_;
    const Facet facet_;
    const BoundingBox facetBoundingBox_;

    Lazy<Vector<>> lazyFacetNormal_;
    Lazy<PlaneTests> lazyPlaneTests_;
    Lazy<AxisDegenerate> lazyAxisDegenerate_;
    Lazy<EdgeTests> lazyEdgeTests_;
};
