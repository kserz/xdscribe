// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/kernel.h"

class Placement final {
public:
    Placement(Point center, double radius)
        : center_(center)
        , radius_(radius)
    {
        assert(radius_ >= 0.);
    }

    const Point& center() const
    {
        return center_;
    }
    double radius() const
    {
        return radius_;
    }

private:
    Point center_;
    double radius_;
};

// By default radius is considered under uniform norm,
// i.e. box is a rectangular one and radius is a half side
using Box = Placement;
