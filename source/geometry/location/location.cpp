// This file is part of xscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "location.h"

#include "geometry/location/axis_distance.h"

#include <set>

Location locatePoint(
        const Point& point,
        Generator<const Facet&> polytopeGeometry)
{
    // We need to merge adjacent faces when counting
    std::set<AxisDistance::Result> upperDistances;

    polytopeGeometry.process([&] (const auto& facet) {
        const auto distance = AxisDistance(facet)(point);
        if (distance && distance.value() > -MEPS) {
            upperDistances.insert(distance);
        }
    });

    if (!upperDistances.empty() &&
            upperDistances.begin()->value() < MEPS) {
        return Location::Boundary;
    } else {
        return upperDistances.size() % 2 == 0 ?
            Location::Outer : Location::Inner;
    }
}

