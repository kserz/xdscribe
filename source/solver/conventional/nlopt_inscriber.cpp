// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "nlopt_inscriber.h"

#include "geometry/entity/bounding_box.h"
#include "geometry/kernel.h"
#include "geometry/location/location.h"
#include "solver/objective.h"

#include <nlopt.hpp>

#include <chrono>
#include <functional>
#include <vector>

namespace {

using Algorithm = NloptInscriber::Algorithm;

nlopt::opt createOptimizator(
        Algorithm algorithm,
        std::optional<double> magic,
        double targetPrecision)
{
    auto result = [&] {
        switch (algorithm) {
        case Algorithm::DirectScaled:
            return nlopt::opt(nlopt::algorithm::GN_DIRECT, DIMS);
        case Algorithm::DirectNonScaled:
            return nlopt::opt(nlopt::algorithm::GN_DIRECT_NOSCAL, DIMS);
        }

        // Should not reach here
        assert(false);
        return nlopt::opt{};
    }();

    // Default value recommended by Jones
    result.set_magic_eps(magic.value_or(targetPrecision));
    return result;
}

Point fromNlopt(const std::vector<double>& x)
{
    assert(x.size() == DIMS);

    Point result;
    for (size_t i = 0; i < DIMS; ++i) {
        result[i] = x[i];
    }

    return result;
}

double objectiveWrapper(
        const std::vector<double> &x,
        std::vector<double>& grad,
        void* data)
{
    assert(grad.empty());

    return (*reinterpret_cast<Objective*>(data))(fromNlopt(x));
}

} // namespace

NloptInscriber::NloptInscriber(
        NloptInscriber::Algorithm algorithm,
        std::optional<double> magic)
    : algorithm_(algorithm)
    , magic_(magic)
{}

Placement NloptInscriber::operator ()(
        const Polytope& pattern,
        const Polytope& contour,
        const StopPredicate& stopPredicate) const
{
    auto opt = createOptimizator(
        algorithm_,
        magic_,
        stopPredicate.targetPrecision);

    Objective objective(&pattern, &contour);

    opt.set_max_objective(&objectiveWrapper, &objective);
    opt.set_lipschitz_constant(objective.lipschitzConstant());

    // TODO: Check why squared box is faster
    const Box container = boundingBox(contour.vertices());
    std::vector<double> lowerBounds(DIMS, std::numeric_limits<double>::max());
    std::vector<double> upperBounds(DIMS, std::numeric_limits<double>::lowest());
    for (size_t i = 0; i < DIMS; ++i) {
        lowerBounds[i] = container.center()[i] - container.radius();
        upperBounds[i] = container.center()[i] + container.radius();
    }
    opt.set_lower_bounds(lowerBounds);
    opt.set_upper_bounds(upperBounds);

    opt.set_xtol_abs(stopPredicate.targetPrecision);
    opt.set_ftol_abs(stopPredicate.targetPrecision);
    opt.set_stopval(stopPredicate.targetValue);
    opt.set_maxtime(std::chrono::duration_cast<std::chrono::seconds>(
        stopPredicate.maxDuration).count());

    std::vector<double> startPoint(
        container.center().data(),
        container.center().data() + DIMS);

    const auto result = opt.optimize(startPoint);

    return Placement{fromNlopt(result), opt.last_optimum_value()};
}
