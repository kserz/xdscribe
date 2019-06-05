// This file is part of xscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include <Eigen/Core>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <limits>
#include <vector>

// We support solutions precision up to 1e-5
// assuming double epsilon around 1e-16.
constexpr double MEPS = 5e-11;
constexpr size_t DIMS = 3;

template<class Scalar = double>
using Vector = Eigen::Matrix<Scalar, DIMS, 1>;

template<class Scalar>
class PointBase final : public Vector<Scalar> {
public:
    static PointBase constant(Scalar value)
    {
        return Vector<Scalar>::Constant(DIMS, value).eval();
    }

    PointBase()
        : PointBase(constant(0))
    {}
    PointBase(std::initializer_list<Scalar> values)
        : Vector<Scalar>(values.begin())
    {
        assert(values.size() == DIMS);
    }

    PointBase(const Vector<Scalar>& other)
        : Vector<Scalar>(other)
    {}
    PointBase(Vector<Scalar>&& other) noexcept
        : Vector<Scalar>(std::move(other))
    {}

    Scalar* begin()
    {
        return this->data();
    }
    Scalar* end()
    {
        return this->data() + this->rows();
    }

    const Scalar* begin() const
    {
        return this->data();
    }
    const Scalar* end() const
    {
        return this->data() + this->rows();
    }
};

using Point = PointBase<double>;
using Coordinates = PointBase<int>;
using Facet = std::array<Point, DIMS>;
using Simplex = std::array<Point, DIMS + 1>;
using Face = std::vector<Point>;

inline int intFloor(double x)
{
    auto result = static_cast<int>(x + MEPS);
    return x < -MEPS ? result - 1 : result;
}
inline Coordinates intFloor(const Point& point)
{
    Coordinates result;
    for (size_t i = 0; i < DIMS; ++i) {
        result[i] = intFloor(point[i]);
    }
    return result;
}

inline bool preceding(const Coordinates& lhs, const Coordinates& rhs)
{
    return std::lexicographical_compare(
        lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
