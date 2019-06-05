// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/entity/polytope.h"
#include "geometry/kernel.h"
#include "utility/noncopyable.h"

#include <Eigen/Dense>

#include <vector>

// No star-shapeness check is performed!
class InscribedRadius final : public NonCopyable {
public:
    static double lipschitzConstant(const Polytope& starShapedPattern);

    InscribedRadius(const Polytope& starShapedPattern, const Polytope& contour);

    double operator ()(const Point& point) const;

private:
    using Face = Polytope::Face;

    // Linear system for finding an intersection of two faces
    // with complementary dimensions.
    // Faces of contour are called fixed,
    // and faces of pattern are of unknown scale, thus scaling.
    //
    // To define the system we consider minimal affine subspaces
    // containing each face. Then affine subspaces intersect if and only if
    // the intersection point can be expressed linearly in each subspace,
    // or equivalently the subspace bases with the origins
    // are linearly dependent collectionwise.
    //
    // So the system here is a linear dependence equation.
    // We put scaling origin as a matrix first column and fixed origin as rhs.
    // This allows us to compute the actual scale leading to intersection as
    // a first coefficient of solution and account pattern placement by
    // on appropriate shifting of fixed origin keeping linear solver untouched.
    //
    // In addition we check the coefficients of the acquired linear combination
    // lately to ensure the intersection point actually lying inside the faces.
    struct IntersectionSystem final {
        using Matrix = Eigen::Matrix<double, DIMS, DIMS>;
        using Solver = Eigen::ColPivHouseholderQR<Matrix>;

        size_t scalingDimension;
        size_t fixedDimension;

        // Note that singular systems mean either no intersection
        // or intersection with any scale.
        // Both cases don't affect the solution,
        // so solver here is always of rank DIMS
        Solver solver;
        Vector<> rhs;
    };

    // Negative scale means no actual intersection
    static double findIntersectionScale(
            const IntersectionSystem& system,
            const Point& scalingOffset);

    static std::vector<IntersectionSystem> precomputeSystems(
            const Polytope& pattern,
            const Polytope& contour);
    static IntersectionSystem intersectionSystem(
            const Face& scalingFace,
            const Face& fixedFace);

    const std::vector<IntersectionSystem> systems_;
};
