// This file is part of xscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/entity/placement.h"
#include "geometry/entity/polytope.h"
#include "utility/noncopyable.h"

#include <optional>
#include <chrono>

using namespace std::chrono_literals;

class Inscriber : public NonCopyable {
public:
    struct StopPredicate final {
        // targetValue should be reachable if it is the only present,
        // otherwise the algorithm will not not stop.
        StopPredicate(
            std::optional<double> targetPrecision,
            std::optional<double> targetValue,
            std::optional<std::chrono::milliseconds> maxDuration)
            : targetPrecision(targetPrecision.value_or(4. * MEPS))
            , targetValue(targetValue.value_or(
                  std::numeric_limits<double>::max()))
            , maxDuration(maxDuration.value_or(72h))
        {
            assert(targetPrecision || targetValue || maxDuration);
            if (this->targetPrecision < 4. * MEPS) {
                throw std::runtime_error("Too small precision specified!");
            }
        }

        bool operator ()(
                double currentPrecision,
                double currentValue,
                const std::chrono::milliseconds& runTime) const
        {
            return runTime > maxDuration ||
                    currentPrecision < targetPrecision ||
                    currentValue > targetValue;
        }

        const double targetPrecision;
        const double targetValue;
        const std::chrono::milliseconds maxDuration;
    };

    virtual ~Inscriber() = default;

    // Result is the center and the radius of the solution
    virtual Placement operator ()(
            const Polytope& pattern,
            const Polytope& contour,
            const StopPredicate& stopPredicate) const = 0;
};
