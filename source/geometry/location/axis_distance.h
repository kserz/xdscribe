// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/entity/bounding_box.h"
#include "geometry/kernel.h"
#include "geometry/location/location.h"
#include "utility/noncopyable.h"

#include <Eigen/Dense>

// Distance along last coordinate axis.
// Allows multiple point queries for a single facet
class AxisDistance final : public NonCopyable {
public:
    class Result final {
    public:
        Result(double value, Location location)
            : value_(value)
            , location_(location)
        {}

        // Distance from the point requested
        double value() const
        {
            return value_;
        }
        // Location of the point projection in the facet
        Location location() const
        {
            return location_;
        }

        explicit operator bool () const
        {
            return location_ != Location::Outer;
        }

        // NB. Distances differing by less then MEPS are considered the same
        bool operator <(const Result& other) const
        {
            return value_ < other.value_ - MEPS;
        }

        bool operator ==(const Result& other) const
        {
            return location_ == other.location_ &&
                    std::fabs(value_ - other.value_) < MEPS;
        }

    private:
        double value_;
        Location location_;
    };

    AxisDistance(const Facet& to);

    Result operator() (const Point& from) const;

private:
    using Matrix = Eigen::Matrix<double, DIMS, DIMS>;

    const BoundingBox boundingBox_;
    const Point origin_;
    Eigen::ColPivHouseholderQR<Matrix> solver_;
};
