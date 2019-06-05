// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "ghj_inscriber.h"

#include "geometry/location/location.h"
#include "helper/stats.h"
#include "grid/rasterization/facet_box_overlap.h"

GHJInscriber::GHJInscriber(ObjectiveBounder objectiveBounder)
    : objectiveBounder_(std::move(objectiveBounder))
{}

GHJInscriber::GHJIteration::GHJIteration(
        Objective objective,
        Subproblem problem)
    : Iteration(
          problem.valueBounds.upper() - problem.valueBounds.lower(),
          Placement{problem.region.center(), problem.valueBounds.lower()})
    , objective(std::move(objective))
{
    subproblems.insert(std::move(problem));
}

std::unique_ptr<IterativeInscriber::Iteration> GHJInscriber::init(
        const Polytope* pattern,
        const Polytope* contour) const
{
    Objective objective(pattern, contour);
    const Box container = boundingBox(contour->vertices());
    Region region{
        container.center(),
        Vector<>::Constant(DIMS, container.radius() * 2.)
    };
    ValueBounds valueBounds = objectiveBounder_(objective, region);

    return std::make_unique<GHJIteration>(
        std::move(objective),
        Subproblem{
            std::move(region),
            std::move(valueBounds)
        });
}

std::unique_ptr<IterativeInscriber::Iteration> GHJInscriber::iterate(
        std::unique_ptr<IterativeInscriber::Iteration> iteration) const
{
    auto& it = dynamic_cast<GHJIteration&>(*iteration);

    Stats::instance().samplingSize.report(it.subproblems.size());
    assert(!it.subproblems.empty());
    Subproblem activeProblem = *it.subproblems.begin();
    it.subproblems.erase(it.subproblems.begin());

    for (auto&& subproblem :
            branchAndBound(activeProblem, it.objective)) {
        if (subproblem.valueBounds.upper() < it.solution.radius()) {
            continue;
        }

        if (subproblem.valueBounds.lower() > it.solution.radius()) {
            it.solution = {
                subproblem.region.center(),
                subproblem.valueBounds.lower()
            };
        }

        it.subproblems.insert(std::move(subproblem));
    }

    it.precision =
        it.subproblems.begin()->valueBounds.upper() - it.solution.radius();

    return std::move(iteration);
}

std::array<GHJInscriber::Subproblem, 3> GHJInscriber::branchAndBound(
        const GHJInscriber::Subproblem& problem,
        const Objective& objective) const
{
    Vector<>::Index branchingAxis = 0;
    Vector<>::Index columnId = 0;
    const auto branchLength =
        problem.region.size().maxCoeff(&branchingAxis, &columnId) / 3.;
    assert(columnId == 0);

    std::array<double, 3> subproblemOffsets = {
        -branchLength,
        0.,
        branchLength
    };

    std::array<Subproblem, 3> result;
    for (size_t i = 0; i < 3; ++i) {
        auto newSize = problem.region.size();
        newSize[branchingAxis] = branchLength;

        result[i].region = Region{
            (problem.region.center() +
                subproblemOffsets[i] * Vector<>::Unit(branchingAxis)).eval(),
            std::move(newSize)
        };

        if (i == 1) {
            result[i].valueBounds = {
                problem.valueBounds.lower(),
                problem.valueBounds.lower() +
                    (problem.valueBounds.upper() - problem.valueBounds.lower()) *
                    result[i].region.size().norm() / problem.region.size().norm()
            };
        } else {
            result[i].valueBounds = objectiveBounder_(
                objective, result[i].region);
        }
    }
    return result;
}
