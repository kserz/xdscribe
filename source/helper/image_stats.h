// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "utility/noncopyable.h"

#include <cstddef>
#include <numeric>
#include <vector>

template<class>
class Sampling;
enum class Location;

class ImageStats final : public NonCopyable {
public:
    enum ImageType {
        Undefined = 0,
        MostlyBoundary,
        MostlyEmpty,
        MostlyFilled,
        FullyBoundary,
        FullyEmpty,
        FullyFilled,

        LAST
    };

    ImageStats()
        : imageTypesCount_(static_cast<size_t>(ImageType::LAST), 0)
    {}

    void report(ImageType imageType)
    {
        ++imageTypesCount_[static_cast<size_t>(imageType)];
    }

    double percentage(ImageType imageType) const
    {
        double sum = std::accumulate(
            imageTypesCount_.begin(), imageTypesCount_.end(), 0);
        return static_cast<double>(
            imageTypesCount_[static_cast<size_t>(imageType)]) / sum;
    }
private:
    std::vector<size_t> imageTypesCount_;
};

void reportSamplingDistribution(const Sampling<Location>& sampling);
