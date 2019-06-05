// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "inscribed_radius.h"

#include "geometry/entity/perpendicular.h"
#include "geometry/location/location.h"
#include "helper/stats.h"

#include <Eigen/Dense>
#include <vector>

double InscribedRadius::lipschitzConstant(const Polytope& starShapedPattern)
{
    double result = 0.;

    starShapedPattern.facetGeometries().process([&] (const Facet& face) {
        auto normal = unitNormal(face);
        // Scaling the pattern by such a value
        // moves every face over 1 unit along the normal
        // thus filling all the points closer than 1 unit
        // to the pattern boundary.
        double scale = 1. / normal.dot(face[0]);
        result = std::max(result, std::fabs(scale));
    });

    return result;
}

InscribedRadius::InscribedRadius(
        const Polytope& starShapedPattern,
        const Polytope& contour)
    : systems_(precomputeSystems(starShapedPattern, contour))
{}

double InscribedRadius::operator ()(const Point& point) const
{
    double minScale = -1.;
    for (const auto& system : systems_) {
        double scale = findIntersectionScale(system, point);
        if (scale < -MEPS) {
            continue;
        } else if (scale < MEPS) {
            // Ensure result to be always non-negative
            return 0.;
        }

        if (minScale < 0. || scale < minScale) {
            minScale = scale;
        }
    }

    // Nontrivial polytopes should intersect at some scale
    assert(minScale > 0.);

    ++Stats::instance().objectiveCalls;

    return minScale;
}

double InscribedRadius::findIntersectionScale(
        const IntersectionSystem& system,
        const Point& scalingOffset)
{
    // Opposite shifting of fixed face origin
    Vector<> rhs = system.rhs - scalingOffset;

    auto solution = system.solver.solve(rhs).eval();
    double scale = solution[0];
    if (scale < -MEPS) {
        return -1.;
    }

    if (scale > MEPS) {
        for (size_t i = 0; i < system.scalingDimension; ++i) {
            solution[1 + i] /= scale;
        }

        if (locationInFace(solution.data() + 1, system.scalingDimension)
                == Location::Outer) {
            return -1.;
        }
    } else {
        // Singular solution: whole scaling face is a single point
        // with all the coordinates equal to 0.
        // But the solver is of full rank, so all the scaling coefficients
        // should be zero in order for the fixed face to pass through
        // the zero point.
        for (size_t i = 0; i < system.scalingDimension; ++i) {
            if (std::fabs(solution[1 + i]) > MEPS) {
                return -1;
            }
        }
    }

    if (locationInFace(
            solution.data() + 1 + system.scalingDimension,
            system.fixedDimension) == Location::Outer) {
        return -1.;
    }

    return scale;
}

std::vector<InscribedRadius::IntersectionSystem>
InscribedRadius::precomputeSystems(
        const Polytope& pattern,
        const Polytope& contour)
{
    std::vector<IntersectionSystem> result;

    for (size_t scalingDim = 0; scalingDim < DIMS; ++scalingDim) {
        pattern.faces(scalingDim).process(
                [&] (const Face& patternFace) {
            contour.faces(DIMS - 1 - scalingDim).process(
                    [&] (const Face& contourFace) {
                auto system = intersectionSystem(patternFace, contourFace);
                // See solver description
                if (system.solver.rank() == DIMS) {
                    result.push_back(std::move(system));
                }
            });
        });
    }

    Stats::instance().geometryElementsCount.report(result.size());

    return result;
}

InscribedRadius::IntersectionSystem InscribedRadius::intersectionSystem(
        const Face& scalingFace,
        const Face& fixedFace)
{
    assert(fixedFace.size() + scalingFace.size() == DIMS + 1);

    IntersectionSystem result;
    result.scalingDimension = scalingFace.size() - 1;
    result.fixedDimension = fixedFace.size() - 1;

    // See intersection system description
    Eigen::Matrix<double, DIMS, DIMS> A;
    A.col(0) = scalingFace[0];
    for (size_t i = 1; i < scalingFace.size(); ++i) {
        A.col(i) = scalingFace[i] - scalingFace[0];
    }
    // Solution components are actually the coordinates of
    // the intersection point over each face basis if the system is thought
    // as an equation of the intersection point expressions.
    // For this to hold the fixed face should be on the right side of
    // the equation along with the origin, meaning opposite sign in the system.
    for (size_t i = 1; i < fixedFace.size(); ++i) {
        A.col(scalingFace.size() + i - 1) = fixedFace[0] - fixedFace[i];
    }

    result.solver = A.colPivHouseholderQr();
    result.solver.setThreshold(MEPS);
    result.rhs = fixedFace[0];

    return result;
}
