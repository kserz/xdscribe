// This file is part of xscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/kernel.h"

#define CGAL_EIGEN3_ENABLED
#include <CGAL/Epick_d.h>

using Kernel = CGAL::Epick_d<CGAL::Dimension_tag<DIMS>>;

inline Kernel::Point_d toCGAL(const Point& p)
{
    return Kernel::Point_d(p.begin(), p.end());
}

inline Point fromCGAL(const Kernel::Point_d& p)
{
    Point result;
    for (size_t i = 0; i < DIMS; ++i) {
        result[i] = p[i];
    }
    return result;
}
