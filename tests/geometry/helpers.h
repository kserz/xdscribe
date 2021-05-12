#pragma once

#include "geometry/kernel.h"

#include <cmath>

inline bool pointsEqual(const Point& lhs, const Point& rhs)
{
    for (size_t coordinate = 0; coordinate < DIMS; ++coordinate) {
        if (std::fabs(lhs[coordinate] - rhs[coordinate]) > 1e-3) {
            return false;
        }
    }
    return true;
}

inline bool facetsEqual(const Facet& lhs, const Facet& rhs)
{
    for (size_t pointIndex = 0; pointIndex < DIMS; ++pointIndex) {
        if (!pointsEqual(lhs[pointIndex], rhs[pointIndex])) {
            return false;
        }
    }
    return true;
}
