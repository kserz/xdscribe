// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "grid/sampling/mapper.h"
#include "grid/sampling/sparse_raster.h"

template<class Value>
class Sampling : public virtual SparseRaster<Value>, public Mapper {
public:
    virtual ~Sampling() = default;

protected:
    Sampling(Mapper mapper)
        : Mapper(std::move(mapper))
    {}
};
