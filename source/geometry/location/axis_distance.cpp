// This file is part of xscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "axis_distance.h"

AxisDistance::AxisDistance(const Facet& to)
    : boundingBox_(boundingBox(to))
    , origin_(to[0])
{
    // System for finding the coordinates over facetBasis
    // of the ray-plane intersection point,
    // see Möller-Trumbore intersection algorithm.
    // Ray is collinear with the last axis.
    Matrix A;
    A.col(0) = Vector<>::Zero();
    A(DIMS-1, 0) = -1.;
    for (size_t i = 1; i < DIMS; ++i) {
        A.col(i) = to[i] - origin_;
    }
    solver_ = A.colPivHouseholderQr();
    solver_.setThreshold(MEPS);
}

AxisDistance::Result AxisDistance::operator() (const Point& from) const
{
    for (size_t i = 0; i < DIMS-1; ++i) {
        if (from[i] < boundingBox_.min()[i] ||
                from[i] > boundingBox_.max()[i]) {
            return {-1., Location::Outer};
        }
    }
    // We skip vertical facets assuming closed polytopes
    if (solver_.rank() < static_cast<int>(DIMS)) {
        return {-1., Location::Outer};
    }

    Vector<> solution = solver_.solve(from - origin_);
    return {
        solution[0],
        locationInFace(solution.data() + 1, DIMS - 1)
    };
}
