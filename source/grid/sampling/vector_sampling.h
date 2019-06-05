// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "grid/sampling/sampling.h"
#include "grid/sampling/vector_sparse_raster.h"
#include "helper/stats.h"

template<class Value>
class VectorSampling final :
        public VectorSparseRaster<Value>,
        public Sampling<Value>
{
public:
    using Voxel = typename VectorSparseRaster<Value>::Voxel;
    using Raster = VectorSparseRaster<Value>;

    VectorSampling(
        Mapper mapper,
        VectorSparseRaster<Value> contents)
        : VectorSparseRaster<Value>(std::move(contents))
        , Sampling<Value>(std::move(mapper))
    {
        Stats::instance().gridSize.report(this->gridSize());
    }
    // Fully filled sampling
    VectorSampling(
        Box container,
        size_t gridSize,
        Value value)
        : VectorSampling(
              Mapper{std::move(container), gridSize},
              VectorSparseRaster{
                  Coordinates::constant(gridSize),
                  value
              })
    {}
};
