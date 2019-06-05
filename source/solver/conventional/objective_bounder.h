// This file is part of xscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/kernel.h"
#include "solver/objective.h"

#include <functional>

class Region final {
public:
    Region(Point center, Vector<> size)
        : center_(std::move(center))
        , size_(std::move(size))
    {}

    const Point& center() const
    {
        return center_;
    }
    const Vector<>& size() const
    {
        return size_;
    }

private:
    Point center_;
    Vector<> size_;
};

class ValueBounds final {
public:
    ValueBounds(double lower, double upper)
        : lower_(lower)
        , upper_(upper)
    {}

    double lower() const
    {
        return lower_;
    }
    double upper() const
    {
        return upper_;
    }

private:
    double lower_;
    double upper_;
};

using ObjectiveBounder = std::function<ValueBounds(
    const Objective& objective,
    const Region& region)>;

inline ValueBounds computeSimpleBounds(
        const Objective& objective,
        const Region& region)
{
    const auto value = objective(region.center());
    return {
        value,
        value + 0.5 * objective.lipschitzConstant() * region.size().norm()
    };
}
