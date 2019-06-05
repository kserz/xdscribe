// This file is part of xscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "image_stats.h"

#include "geometry/location/location.h"
#include "helper/stats.h"
#include "grid/sampling/sampling.h"

#include <array>

void reportSamplingDistribution(const Sampling<Location>& sampling)
{
    size_t samplingSize = 0;
    std::array<size_t, 3> locationCounters = {0, 0, 0};

    sampling.voxels().process([&] (const auto& voxel) {
        ++samplingSize;
        ++locationCounters[static_cast<int>(voxel.value)];
    });

    Stats::instance().samplingSize.report(samplingSize);
    Stats::instance().samplingToGridRatio.report(
        static_cast<double>(samplingSize) /
        rasterCapacity(sampling.rasterSize()));

    std::array<ImageStats::ImageType, 3> mostlyTypes = {
        ImageStats::ImageType::MostlyEmpty,
        ImageStats::ImageType::MostlyBoundary,
        ImageStats::ImageType::MostlyFilled
    };
    std::array<ImageStats::ImageType, 3> fullyTypes = {
        ImageStats::ImageType::FullyEmpty,
        ImageStats::ImageType::FullyBoundary,
        ImageStats::ImageType::FullyFilled
    };

    bool undefined = true;
    for (size_t i = 0; i < 3; ++i) {
        if (locationCounters[i] > samplingSize * 2 / 3) {
            Stats::instance().images.report(fullyTypes[i]);
            undefined = false;
        } else if (locationCounters[i] > samplingSize / 2) {
            Stats::instance().images.report(mostlyTypes[i]);
            undefined = false;
        }
    }

    if (undefined) {
        Stats::instance().images.report(ImageStats::ImageType::Undefined);
    }
}
