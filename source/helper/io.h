// This file is part of xscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/kernel.h"
#include "geometry/location/location.h"
#include "grid/sampling/sampling.h"
#include "utility/generator.h"

#include <iostream> // To provide std::cout, std::cerr immediately
#include <string>
#include <vector>

template<class Scalar>
std::ostream& operator <<(std::ostream& out, const PointBase<Scalar>& point)
{
    out << "( ";
    for (const auto& coordinate : point) {
        out << coordinate << " ";
    }
    out << ")";

    return out;
}

std::ostream& operator <<(std::ostream& out, Location location);

// Draw a 3d model in the Wavefront OBJ format
void drawPoints(
        const std::string& filename,
        const std::vector<Point>& points);
void drawFacets(
        const std::string& filename,
        const Generator<const Facet&>& facets);
void drawSampling(
        const std::string& filename,
        const Sampling<Location>& sampling);

// Dump layers in a matrix text file
void dumpSampling(
        const std::string& filename,
        const Sampling<Location>& sampling);
