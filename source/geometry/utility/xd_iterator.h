// This file is part of xscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/kernel.h"

#include <cstddef>
#include <functional>

template<size_t dims = DIMS>
class XDIterator final {
public:
    // TODO: look at the performance for different callback schemes
    using Callback = std::function<void(const Coordinates&)>;

    // Only first dims coordinates of size are used
    // Zeros are always passed to callback in the last DIMS-dims coordinates
    static inline void run(const Coordinates& size, const Callback& callback)
    {
        assert(dims <= DIMS);
        auto coordinates = Coordinates::constant(0);
        XDIterator<dims>::iterate(&coordinates, size, callback);
    }

private:
    template<size_t otherDims> friend class XDIterator;

    static inline void iterate(
            Coordinates* coordinates,
            const Coordinates& size,
            const Callback& callback);
};

template<size_t dims>
inline void XDIterator<dims>::iterate(
        Coordinates* coordinates,
        const Coordinates& size,
        const Callback& callback)
{
    // TODO: Check speed of reverse ordering
    for (int i = 0; i < size[dims-1]; ++i) {
        (*coordinates)[dims-1] = i;
        XDIterator<dims-1>::iterate(coordinates, size, callback);
    }
}

template<>
inline void XDIterator<0>::iterate(
        Coordinates* coordinates,
        const Coordinates& /*size*/,
        const Callback& callback)
{
    callback(*coordinates);
}
