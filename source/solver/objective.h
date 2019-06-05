// This file is part of xscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/entity/polytope.h"
#include "geometry/kernel.h"
#include "geometry/location/location.h"
#include "solver/inscribed_radius.h"
#include "utility/noncopyable.h"

// Lipschitz-continuous extension
// of the inscribed radius value inside the contour
class Objective final : public NonCopyable {
public:
    Objective(const Polytope* pattern, const Polytope* contour)
        : inscribedRadius_(*pattern, *contour)
        , contour_(contour)
        , lipschitzConstant_(InscribedRadius::lipschitzConstant(*pattern))
    {}

    double operator ()(const Point& point) const
    {
        if (locatePoint(point, contour_->facetGeometries())
                == Location::Inner) {
            return inscribedRadius_(point);
        } else {
            return 0.;
        }
    }

    double lipschitzConstant() const
    {
        return lipschitzConstant_;
    }

    const Polytope* contour() const
    {
        return contour_;
    }

private:
    InscribedRadius inscribedRadius_;
    const Polytope* const contour_;
    const double lipschitzConstant_;
};
