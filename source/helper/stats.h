// This file is part of xscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "helper/image_stats.h"
#include "utility/noncopyable.h"

#include <algorithm>
#include <cstddef>
#include <map>
#include <ostream>
#include <string>

template<class Value>
class ValueStats final : public NonCopyable {
public:
    ValueStats()
        : max_(0)
        , sum_(0)
        , count_(0)
    {}

    void report(Value value)
    {
        max_ = std::max(max_, value);
        sum_ += value;
        count_ += 1;
    }

    Value max() const
    {
        return max_;
    }
    Value average() const
    {
        return count_ > 0 ? sum_ / count_ : 0;
    }

private:
    Value max_;
    Value sum_;
    size_t count_;
};

struct Stats final : public NonCopyable {
    using ImageType = ImageStats::ImageType;

    class PolytopeFaceKey final {
    public:
        PolytopeFaceKey(std::string name, size_t dims)
            : name_(std::move(name))
            , dims_(dims)
        {}

        const std::string& name() const
        {
            return name_;
        }
        size_t dims() const
        {
            return dims_;
        }

        bool operator < (const PolytopeFaceKey& rhs) const {
            if (name_ != rhs.name_) {
                return name_ < rhs.name_;
            } else {
                return dims_ < rhs.dims_;
            }
        }

    private:
        std::string name_;
        size_t dims_;
    };

    static Stats& instance()
    {
        static Stats instance_;
        return instance_;
    }

    std::map<PolytopeFaceKey, size_t> polytopeFacesCount;
    size_t patternConvexPartsCount;
    ValueStats<size_t> geometryElementsCount;

    ValueStats<size_t> gridSize;
    ValueStats<size_t> samplingSize;
    ValueStats<double> samplingToGridRatio;

    ImageStats images;

    size_t inscriberSteps = 0;
    size_t objectiveCalls = 0;
};

std::ostream& operator <<(std::ostream& out, ImageStats::ImageType imageType);
