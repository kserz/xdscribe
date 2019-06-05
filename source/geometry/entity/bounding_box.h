// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/entity/placement.h"
#include "geometry/kernel.h"
#include "utility/noncopyable.h"

class BoundingBox final : public NonCopyable {
public:
    BoundingBox(Point min, Point max)
        : min_(std::move(min))
        , max_(std::move(max))
    {}

    const Point& min() const
    {
        return min_;
    }
    const Point& max() const
    {
        return max_;
    }

    operator Box() const
    {
        return {
            ((max_ + min_) / 2.).eval(),
            (max_ - min_).maxCoeff() / 2.
        };
    }

private:
    const Point min_;
    const Point max_;
};

template<class Container>
inline BoundingBox boundingBox(const Container& points)
{
    assert(points.size() > 0);

    Point min{points[0]};
    Point max{points[0]};

    for (size_t j = 1; j < points.size(); ++j) {
        for (size_t i = 0; i < DIMS; ++i) {
            const auto coordinate = points[j][i];
            min[i] = std::min(min[i], coordinate);
            max[i] = std::max(max[i], coordinate);
        }
    }

    return {min, max};
}
