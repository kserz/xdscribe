// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "solver/conventional/objective_bounder.h"
#include "solver/iterative_inscriber.h"
#include "solver/objective.h"

#include <array>
#include <memory>
#include <set>

class GHJInscriber final : public IterativeInscriber {
public:
    GHJInscriber(ObjectiveBounder objectiveBounder);

private:
    struct Subproblem final {
        Subproblem(Region region, ValueBounds valueBounds)
            : region(std::move(region))
            , valueBounds(std::move(valueBounds))
        {}
        Subproblem()
            : region(Point::constant(0.), Vector<>::Constant(0.))
            , valueBounds(0., 0.)
        {}

        Region region;
        ValueBounds valueBounds;

        // We split subproblems with the biggest value first
        bool operator <(const Subproblem& rhs) const
        {
            return valueBounds.upper() > rhs.valueBounds.upper();
        }
    };

    struct GHJIteration final : public Iteration {
        GHJIteration(Objective objective, Subproblem problem);

        const Objective objective;
        std::multiset<Subproblem> subproblems;
    };

    virtual std::unique_ptr<Iteration> init(
            const Polytope* pattern,
            const Polytope* contour) const override;
    virtual std::unique_ptr<Iteration> iterate(
        std::unique_ptr<Iteration> previous) const override;

    std::array<Subproblem, 3> branchAndBound(
            const Subproblem& problem,
            const Objective& objective) const;

    ObjectiveBounder objectiveBounder_;
};
