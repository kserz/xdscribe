// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "halfspace_part_rasterizer.h"

#include "geometry/entity/perpendicular.h"
#include "grid/rasterization/facet_box_overlap.h"
#include "grid/rasterization/facet_rasterizer.h"

namespace {

struct Halfspace {
    Vector<> normal;
    double planeOffset;
};

Halfspace outsideOrientedHalfspace(
        const Facet& localFacet,
        const Facet& baseFacet,
        const Vector<> innerDirection)
{
    Halfspace result;
    result.normal = unitNormal(localFacet);
    result.planeOffset = result.normal.dot(localFacet[0]);

    double innerOffset = 0.;
    for (size_t i = 0; std::fabs(innerOffset) < MEPS && i < DIMS; ++i) {
        innerOffset = result.normal.dot(baseFacet[i]) - result.planeOffset;
    }
    if (std::fabs(innerOffset) < MEPS) {
        innerOffset = result.normal.dot(innerDirection);
    }
    assert(std::fabs(innerOffset) > MEPS);

    if (innerOffset > 0.) {
        result.normal = -result.normal;
        result.planeOffset = -result.planeOffset;
    }

    return result;
}

}

void rasterizePartByHalfspaces(
        const MinkowskiSum::ConvexPart& convexPart,
        Sampling<Location>* partSampling)
{
    partSampling->voxels().process([] (auto& voxel) {
        voxel.value = Location::Inner;
    });

    const auto baseFacet = partSampling->toLocal(convexPart.baseFacet);
    partSampling->toLocal(convexPart.facets).process(
                [&] (const auto& localFacet) {
        auto halfspace = outsideOrientedHalfspace(
            localFacet, baseFacet, convexPart.innerDirection);

        // Voxels are unit cubes in local coordinates
        const auto lowestPoint = FacetBoxOverlap::lowestPoint(
            Vector<>::Constant(1.), halfspace.normal);
        const double voxelSizeProjection = halfspace.normal.cwiseAbs().sum();

        partSampling->voxels().process([&] (Voxel<Location>& voxel) {
            double signedPlaneDistance = halfspace.normal.dot(
                voxel.coordinates().cast<double>() + lowestPoint)
                    - halfspace.planeOffset;

            auto& value = voxel.value;
            if (signedPlaneDistance > MEPS) {
                value = Location::Outer;
            } else if (signedPlaneDistance + voxelSizeProjection > -MEPS &&
                    value != Location::Outer) {
                value = Location::Boundary;
            }
        });
    });
}
