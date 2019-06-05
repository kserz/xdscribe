// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "solver/inscriber.h"
#include "utility/noncopyable.h"

#include <memory>

class IterativeInscriber : public Inscriber {
public:
    virtual Placement operator ()(
            const Polytope& pattern,
            const Polytope& contour,
            const StopPredicate& stopPredicate) const override;

protected:
    struct Iteration {
        Iteration(double precision, Placement solution)
            : precision(precision)
            , solution(std::move(solution))
        {}

        virtual ~Iteration() = default;

        double precision;
        Placement solution;
    };

    virtual std::unique_ptr<Iteration> init(
            const Polytope* pattern,
            const Polytope* contour) const = 0;
    // Iterations should be as short as possible
    // to produce the best result within time limits
    virtual std::unique_ptr<Iteration> iterate(
        std::unique_ptr<Iteration> previous) const = 0;
};
