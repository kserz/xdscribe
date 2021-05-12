// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/kernel.h"
#include "grid/sampling/xd_iterator.h"
#include "utility/generator.h"

#include <functional>
#include <memory>

template<class Value>
class Voxel final {
public:
    Voxel(Coordinates coordinates, Value value)
        : coordinates_(std::move(coordinates))
        , value(std::move(value))
    {}

    const Coordinates& coordinates() const
    {
        return coordinates_;
    }

private:
    Coordinates coordinates_;

public:
    // Ordering of fields here impacts performance a bit
    Value value;
};

// Subset of values on a rectangular grid
template<class Value>
class SparseRaster {
public:
    using Voxel = ::Voxel<Value>;

    virtual ~SparseRaster() = default;

    // Actual stored data size
    virtual size_t size() const = 0;

    virtual Generator<Voxel&> voxels() = 0;
    virtual Generator<const Voxel&> voxels() const = 0;

    // Return nullptr in case of absence
    virtual Voxel* find(const Coordinates& coordinates) = 0;
    virtual const Voxel* find(const Coordinates& coordinates) const = 0;

    // All the voxels along the last axis direction
    // starting with a specified point
    virtual Generator<Voxel&> verticalSlice(const Coordinates& start) = 0;
};

inline size_t rasterCapacity(const Coordinates& rasterSize)
{
    size_t result = 1;
    for (const auto& c : rasterSize) {
        assert(c > 0);
        result *= c;
    }
    return result;
}
