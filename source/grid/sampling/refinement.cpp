// This file is part of xscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "refinement.h"

#include "geometry/entity/bounding_box.h"

#include <limits>

Box voxelsBoundingBox(const std::vector<Coordinates>& selection)
{
    auto min = Coordinates::constant(std::numeric_limits<int>::max());
    auto max = Coordinates::constant(0);

    for (const auto& coordinates : selection) {
        for (size_t i = 0; i < DIMS; ++i) {
            min[i] = std::min(min[i], coordinates[i]);
            max[i] = std::max(max[i], coordinates[i]);
        }
    }

    max += Coordinates::constant(1);
    // Align the box to voxel boundaries
    // by making center and radius integer
    for (size_t i = 0; i < DIMS; ++i) {
        assert(max[i] > min[i]);
        if ((max[i] - min[i]) % 2 != 0) {
            ++max[i];
        }
    }

    return BoundingBox{
        min.cast<double>().eval(),
        max.cast<double>().eval()
    };
}
