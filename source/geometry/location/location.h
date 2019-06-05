// This file is part of xscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/kernel.h"
#include "utility/generator.h"

enum class Location {
    Outer = 0,
    Boundary,
    Inner
};

Location locatePoint(
        const Point& point,
        Generator<const Facet&> polytopeGeometry);

// baseCoordinates are the coordinates of a test point
// over the basis induced by face
inline Location locationInFace(
        const double* baseCoordinates,
        size_t faceDimension)
{
    double sum = 0.;
    bool boundary = false;

    for (size_t i = 0; i < faceDimension; ++i) {
        const auto& coordinate = baseCoordinates[i];
        if (coordinate < -MEPS || coordinate > 1. + MEPS) {
            return Location::Outer;
        } else if (coordinate < MEPS || coordinate > 1. - MEPS) {
            boundary = true;
        }
        sum += coordinate;
    }

    if (sum > 1. + MEPS) {
        return Location::Outer;
    } else if (sum > 1. - MEPS) {
        boundary = true;
    }

    return boundary ? Location::Boundary : Location::Inner;
}
