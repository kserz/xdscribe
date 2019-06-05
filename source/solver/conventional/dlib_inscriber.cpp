// This file is part of xscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "dlib_inscriber.h"

#include "geometry/entity/bounding_box.h"
#include "geometry/kernel.h"
#include "geometry/location/location.h"
#include "solver/objective.h"

#include <dlib/global_optimization.h>
#include <dlib/optimization.h>

namespace {

using DlibPoint = dlib::matrix<double, 0, 1>;

Point fromDlib(const DlibPoint& input)
{
    assert(input.nr() == DIMS);
    assert(input.nc() == 1);

    Point result;
    for (size_t i = 0; i < DIMS; ++i) {
        result[i] = input(i, 0);
    }

    return result;
}

} // namespace

Placement DlibInscriber::operator ()(
        const Polytope& pattern,
        const Polytope& contour,
        const StopPredicate& stopPredicate) const
{
    const Objective objective(&pattern, &contour);

    const auto box = boundingBox(contour.vertices());
    dlib::matrix<double, 0, 1> lowerBounds =
        dlib::uniform_matrix(DIMS, 1, std::numeric_limits<double>::max());
    dlib::matrix<double, 0, 1> upperBounds =
        dlib::uniform_matrix(DIMS, 1, std::numeric_limits<double>::lowest());
    for (size_t i = 0; i < DIMS; ++i) {
        lowerBounds(i, 0) = box.min()[i];
        upperBounds(i, 0) = box.max()[i];
    }

    auto result = dlib::find_max_global(
        [&] (const DlibPoint& input) {
            return objective(fromDlib(input));
        },
        lowerBounds,
        upperBounds,
        dlib::max_function_calls(),
        stopPredicate.maxDuration,
        dlib::function_value(stopPredicate.targetValue),
        stopPredicate.targetPrecision);

    return {fromDlib(result.x), result.y};
}
