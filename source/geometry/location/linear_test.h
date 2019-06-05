// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/kernel.h"
#include "utility/noncopyable.h"

class LinearTest final : public NonCopyable {
public:
    LinearTest(Vector<> direction, double offset)
        : direction_(std::move(direction))
        , offset_(std::move(offset))
    {}
    LinearTest()
        : LinearTest(Vector<>::Constant(0.), 0.)
    {}

    double operator ()(const Point& point) const
    {
        return direction_.dot(point) + offset_;
    }

private:
    Vector<> direction_;
    double offset_;
};
