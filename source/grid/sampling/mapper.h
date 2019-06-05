// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/entity/placement.h"
#include "geometry/kernel.h"
#include "helper/stats.h"
#include "utility/generator.h"

// Transforms entities between global coordinates
// and grid based (local) ones
class Mapper {
public:
    Mapper(Box container, size_t gridSize)
        : container_(std::move(container))
        , gridSize_(gridSize)
        , gridStep_((2. * container_.radius()) / static_cast<double>(gridSize_))
        , localCenter_(Point::constant(static_cast<double>(gridSize_) / 2.))
        , rasterSize_(Coordinates::constant(gridSize_))
    {
        Stats::instance().gridSize.report(gridSize_);
    }
    virtual ~Mapper() = default;

    const Box& container() const
    {
        return container_;
    }
    size_t gridSize() const
    {
        return gridSize_;
    }
    double gridStep() const
    {
        return gridStep_;
    }
    const Coordinates& rasterSize() const
    {
        return rasterSize_;
    }

    bool contains(const Coordinates& coordinates) const
    {
        for (size_t i = 0; i < DIMS; ++i) {
            if (coordinates[i] < 0 || coordinates[i] >= rasterSize_[i]) {
                return false;
            }
        }
        return true;
    }

    double toLocal(double distance) const
    {
        return distance / gridStep_;
    }
    Point toLocal(const Point& point) const
    {
        return Point(localCenter_ +
            (point - container_.center()) / gridStep_);
    }
    Facet toLocal(const Facet& face) const
    {
        Facet result;
        for (size_t i = 0; i < DIMS; ++i) {
            result[i] = toLocal(face[i]);
        }
        return result;
    }
    Generator<const Facet&> toLocal(Generator<const Facet&> geometry) const
    {
        return mapGenerator<const Facet&>(
            std::move(geometry),
            [this] (const Facet& facet) {
                return toLocal(facet);
            });
    }

    double toGlobal(double distance) const
    {
        return distance * gridStep_;
    }
    Point toGlobal(const Coordinates& coordinates) const
    {
        return toGlobal(Point(coordinates.cast<double>()));
    }
    Point toGlobal(const Point& point) const
    {
        return Point(container_.center() +
            (point - localCenter_) * gridStep_);
    }
    Box toGlobal(const Box& box) const
    {
        return {
            toGlobal(box.center()),
            toGlobal(box.radius())
        };
    }

protected:
    Box container_;
    double gridSize_;
    double gridStep_;
    Point localCenter_;
    Coordinates rasterSize_;
};
