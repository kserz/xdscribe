// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "inner_region_rasterizer.h"

#include "geometry/location/axis_distance.h"
#include "grid/sampling/vector_sparse_raster.h"

#include <vector>

namespace {

inline Point voxelCenter(const Coordinates& coordinates)
{
    static const Point voxelCenterOffset = Point::constant(0.5);
    return (coordinates.cast<double>() + voxelCenterOffset).eval();
}

VectorSparseRaster<std::vector<AxisDistance::Result>> sortedDistancesAbove(
        const Generator<const Coordinates&>& selection,
        size_t estimatedCapacity,
        const Generator<const Facet&>& localPolytopeGeometry)
{
    VectorSparseRaster<std::vector<AxisDistance::Result>> result(
        selection, {}, estimatedCapacity);

    localPolytopeGeometry.process([&] (const auto& facet) {
        AxisDistance facetDistance(facet);

        result.voxels().process([&] (auto& voxel) {
            auto testPoint = voxelCenter(voxel.coordinates());
            auto distance = facetDistance(testPoint);

            // Solve the cases where the geometry
            // merely touches the vertical ray
            // and the test point is still outside
            size_t offsetCoordinateIndex = 0;
            while (distance.location() == Location::Boundary &&
                    offsetCoordinateIndex < DIMS) {
                testPoint[offsetCoordinateIndex] += 2. * MEPS;
                distance = facetDistance(testPoint);
                ++offsetCoordinateIndex;
            }

            if (distance && distance.value() > -MEPS) {
                voxel.value.push_back(distance);
            }
        });
    });

    result.voxels().process([] (auto& voxel) {
        auto& value = voxel.value;
        std::sort(value.begin(), value.end());
        auto last = std::unique(value.begin(), value.end());
        value.erase(last, value.end());
    });

    return result;
}

} // namespace

void rasterizeInnerRegionSequentally(
        const Generator<const Facet&>& localPolytopeGeometry,
        SparseRaster<Location>* raster)
{
    raster->voxels().process([&] (auto& voxel) {
        if (voxel.value != Location::Outer) {
            return;
        }

        voxel.value = locatePoint(
            voxelCenter(voxel.coordinates()),
            localPolytopeGeometry);
    });
}

void rasterizeInnerRegionByFacets(
        const Generator<const Facet&>& localPolytopeGeometry,
        SparseRaster<Location>* raster)
{
    const auto distancesAbove = sortedDistancesAbove(
        compositeGenerator<const Coordinates&>(
            raster->voxels(),
            [] (const auto& voxel, auto&& yield) {
                if (voxel.value == Location::Outer) {
                    yield(voxel.coordinates());
                }
            }),
        raster->size(),
        localPolytopeGeometry);

    // Iterate over outer nodes only
    distancesAbove.voxels().process([&] (const auto& distancesVoxel) {
        const auto& nodeDistancesAbove = distancesVoxel.value;
        auto* targetVoxel = raster->find(distancesVoxel.coordinates());
        assert(targetVoxel);
        auto& targetVoxelValue = targetVoxel->value;

        if (!nodeDistancesAbove.empty() &&
                nodeDistancesAbove.begin()->value() < MEPS) {
            targetVoxelValue = Location::Boundary;
        } else if (nodeDistancesAbove.size() % 2 != 0){
            targetVoxelValue = Location::Inner;
        }
    });
}

void rasterizeInnerRegionByRays(
        const Generator<const Facet&>& localPolytopeGeometry,
        SparseRaster<Location>* raster)
{
     const auto distancesAbove = sortedDistancesAbove(
        compositeGenerator<const Coordinates&>(
            raster->voxels(),
            [] (const auto& voxel, auto&& yield) {
                if (voxel.value == Location::Outer) {
                    // Projection on 0-plane
                    auto coordinates = voxel.coordinates();
                    coordinates[DIMS-1] = 0;
                    yield(coordinates);
                }
            }),
        raster->size(),
        localPolytopeGeometry);

    distancesAbove.voxels().process([&] (auto& distancesVoxel) {
        const auto& nodeDistancesAbove = distancesVoxel.value;
        auto currentDistanceAbove = nodeDistancesAbove.begin();
        size_t intersectionsLeftAbove = nodeDistancesAbove.size();
        assert (distancesVoxel.coordinates()[DIMS-1] == 0);

        raster->verticalSlice(distancesVoxel.coordinates()).process(
                [&] (auto& samplingVoxel) {
            if (samplingVoxel.value != Location::Outer) {
                return;
            }

            auto currentBound = static_cast<double>(
                samplingVoxel.coordinates()[DIMS-1]);

            while(currentDistanceAbove != nodeDistancesAbove.end() &&
                   currentDistanceAbove->value() < currentBound - MEPS) {
                ++currentDistanceAbove;
                assert(intersectionsLeftAbove > 0);
                --intersectionsLeftAbove;
            }

            if (currentDistanceAbove == nodeDistancesAbove.end()) {
                return;
            }

            if (currentDistanceAbove->value() < currentBound + MEPS) {
                samplingVoxel.value = Location::Boundary;
            } else if (intersectionsLeftAbove % 2 == 1) {
                samplingVoxel.value = Location::Inner;
            }
        });
    });
}
