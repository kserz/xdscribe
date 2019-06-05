// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/kernel.h"
#include "geometry/location/location.h"
#include "utility/noncopyable.h"

// Overlap test for a simplex and multiple facets
class SimplexFacetOverlap final : public NonCopyable {
public:
    SimplexFacetOverlap(const Simplex* simplex);

    // Inner = overlap, Outer = no intersection
    Location operator ()(const Facet& facet) const;

private:
    const Simplex* const simplex_;
};
