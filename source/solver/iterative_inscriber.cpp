// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "iterative_inscriber.h"

#include "helper/stats.h"

#include <chrono>

Placement IterativeInscriber::operator ()(
        const Polytope& pattern,
        const Polytope& contour,
        const Inscriber::StopPredicate& stopPredicate) const
{
    auto startTime = std::chrono::steady_clock::now();
    const auto runTime = [&] () {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - startTime);
    };

    size_t steps = 0;
    auto iteration = init(&pattern, &contour, stopPredicate.targetPrecision);
    auto result = iteration->solution;
    while (!stopPredicate(
               iteration->precision,
               iteration->solution.radius(),
               runTime())) {
        iteration = iterate(std::move(iteration));

        if (runTime() < stopPredicate.maxDuration) {
            result = iteration->solution;
        } else {
            break;
        }

        ++steps;
    }

    Stats::instance().inscriberSteps = steps;
    return result;
}
