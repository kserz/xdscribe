// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "grid/sampling/sparse_raster.h"
#include "grid/sampling/xd_iterator.h"
#include "utility/generator.h"

#include <algorithm>
#include <vector>

template<class Value>
class VectorSparseRaster : public virtual SparseRaster<Value>
{
public:
    using Voxel = typename SparseRaster<Value>::Voxel;

    // Accepts selections of arbitrary order and containing duplicates
    VectorSparseRaster(
            const Generator<const Coordinates&>& selection,
            Value value,
            size_t estimatedCapacity);
    VectorSparseRaster(
            const std::vector<Coordinates>& selection,
            Value value);
    // Fully filled raster
    VectorSparseRaster(
            const Coordinates& rasterSize,
            Value value);

    virtual size_t size() const override
    {
        return sortedSelection_.size();
    }

    Generator<Voxel&> voxels() override
    {
        return Generator<Voxel&>(&sortedSelection_);
    }
    Generator<const Voxel&> voxels() const override
    {
        return Generator<const Voxel&>(&sortedSelection_);
    }

    virtual Voxel* find(const Coordinates &coordinates) override
    {
        auto it = std::lower_bound(
            sortedSelection_.begin(), sortedSelection_.end(), coordinates,
            [] (const auto& voxel, const auto& coordinates) {
                return preceding(voxel.coordinates(), coordinates);
            });

        if (it == sortedSelection_.end() || it->coordinates() != coordinates) {
            return nullptr;
        }

        return &(*it);
    }
    virtual const Voxel* find(const Coordinates& coordinates) const override
    {
        return const_cast<VectorSparseRaster*>(this)->find(coordinates);
    }

    virtual Generator<Voxel&> verticalSlice(const Coordinates &start) override
    {
        return Generator<Voxel&>([this, start] (auto&& yield) {
            auto it = std::lower_bound(
                sortedSelection_.begin(), sortedSelection_.end(), start,
                [] (const auto& voxel, const auto& coordinates) {
                    return preceding(voxel.coordinates(), coordinates);
                });

            auto projectionsEqual = [] (
                    const Coordinates& lhs, const Coordinates& rhs) {
                for (size_t i = 0; i < DIMS-1; ++i) {
                    if (lhs[i] != rhs[i]) {
                        return false;
                    }
                }
                return true;
            };

            while (it != sortedSelection_.end() &&
                    projectionsEqual(it->coordinates(), start)) {
                yield(*it);
                ++it;
            }
        });
    }

protected:
    // Sorting is needed to produce correct slices and find to work
    std::vector<Voxel> sortedSelection_;
};

template<class Value>
VectorSparseRaster<Value>::VectorSparseRaster(
        const Generator<const Coordinates&>& selection,
        Value value,
        size_t estimatedCapacity)
{
    sortedSelection_.reserve(estimatedCapacity);
    selection.process([&] (const auto& coordinates) {
        sortedSelection_.push_back(Voxel{coordinates, value});
    });

    // Note that refinement does not produce selection in the right order
    // and grid_location adds multiple instances of the same voxel
    std::sort(
        sortedSelection_.begin(),
        sortedSelection_.end(),
        [] (const auto& lhs, const auto& rhs) {
            return preceding(lhs.coordinates(), rhs.coordinates());
        });
    auto last = std::unique(
        sortedSelection_.begin(),
        sortedSelection_.end(),
        [] (const auto& lhs, const auto& rhs) {
            return lhs.coordinates() == rhs.coordinates();
        });
    sortedSelection_.erase(last, sortedSelection_.end());
}

template<class Value>
VectorSparseRaster<Value>::VectorSparseRaster(
        const std::vector<Coordinates>& selection,
        Value value)
    : VectorSparseRaster<Value>(
          Generator<const Coordinates&>(&selection),
          value,
          selection.size())
{}

template<class Value>
VectorSparseRaster<Value>::VectorSparseRaster(
        const Coordinates& rasterSize,
        Value value)
    : VectorSparseRaster<Value>(
          Generator<const Coordinates&>([&] (auto&& yield) {
              XDIterator<DIMS>::run(
                  rasterSize,
                  [&] (const Coordinates& coordinates) {
                      yield(coordinates);
                  });
              }),
          value,
          rasterCapacity(rasterSize))
{}
