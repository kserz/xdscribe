// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/kernel.h"

#include <Eigen/Dense>

// Returns one of the possible perpendiculars
template<class LHSFace, class RHSFace>
inline Vector<> commonPerpendicular(const LHSFace& lhs, const RHSFace& rhs)
{
    assert(lhs.size() + rhs.size() == DIMS + 1);
    assert(lhs.size() >= 1 && rhs.size() >= 1);

    // Linear system on orthogonality
    Eigen::Matrix<double, DIMS - 1, DIMS> A;
    for (size_t i = 0; i < lhs.size() - 1; ++i) {
        A.row(i) = lhs[i + 1] - lhs[0];
    }
    for (size_t i = 0; i < rhs.size() - 1; ++i) {
        A.row(lhs.size() - 1 + i) = rhs[i + 1] - rhs[0];
    }

    auto solver = A.fullPivLu();
    solver.setThreshold(MEPS);
    return solver.kernel().col(0);
}

// Returns one of the two possible unit normals
inline Vector<> unitNormal(const Facet& facet)
{
    static const std::vector<Point> zero(1, Point::constant(0));
    auto result = commonPerpendicular(facet, zero);
    result /= result.norm();
    return result;
}
