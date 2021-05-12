// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/entity/placement.h"
#include "geometry/kernel.h"
#include "geometry/location/location.h"
#include "grid/sampling/mapper.h"
#include "grid/sampling/sparse_raster.h"
#include "grid/sampling/xd_iterator.h"

#include <vector>

static const size_t REFINEMENT_SCALE = 2;

// Returns box perfectly aligned to voxel boundaries
Box voxelsBoundingBox(const std::vector<Coordinates>& selection);

// Removes all the empty voxels from the sampling and shrinks container
// Returns sampling filled with boundary(undefined) values.
// NB. Subsequent shrinks/refines should be correct in any order!
template<template<class> class Sampling>
Sampling<Location> shrink(const Sampling<Location>& sampling)
{
    // Selection is iterated twice here
    std::vector<Coordinates> selection;
    selection.reserve(sampling.size());
    sampling.voxels().process([&] (const auto& voxel) {
        if (voxel.value != Location::Outer) {
            selection.push_back(voxel.coordinates());
        }
    });

    const auto localContainer = voxelsBoundingBox(selection);
    const size_t gridSize = static_cast<size_t>(
        intFloor(localContainer.radius() * 2.));
    const Mapper mapper(sampling.toGlobal(localContainer), gridSize);

    // Ensure voxels of the old & new grids aligned
    // to safely transfer coordinates
    assert(std::fabs(mapper.toGlobal(1.) - sampling.toGlobal(1.)) < MEPS);
    assert(gridSize % 2 == 0);

    const auto offset = (intFloor(localContainer.center()) -
        Coordinates::constant(gridSize / 2)).eval();

    for (auto& coordinates : selection) {
        auto resultCoordinates = (coordinates - offset).eval();
        assert(mapper.contains(resultCoordinates));
        coordinates = std::move(resultCoordinates);
    }

    return {
        mapper,
        typename Sampling<Location>::Raster{
            selection,
            Location::Boundary
        }
    };
}

// Refines non-empty part of sampling
// Returns sampling filled with boundary(undefined) values
// NB. Subsequent shrinks/refines should be correct in any order!
template<template<class> class Sampling>
Sampling<Location> refine(const Sampling<Location>& sampling)
{
    auto selection = compositeGenerator<const Coordinates&>(
        sampling.voxels(),
        [&] (const auto& voxel, auto&& yield) {
            if (voxel.value == Location::Outer) {
                return;
            }

            XDIterator<DIMS>::run(
                Coordinates::constant(REFINEMENT_SCALE),
                [&] (const Coordinates& offset) {
                    yield((voxel.coordinates() * REFINEMENT_SCALE
                       + offset).eval());
                });
        });

    return {
        Mapper{sampling.container(), sampling.gridSize() * REFINEMENT_SCALE},
        typename Sampling<Location>::Raster{
            selection,
            Location::Boundary,
            sampling.size() *
                rasterCapacity(Coordinates::constant(REFINEMENT_SCALE))
        }
    };
}
