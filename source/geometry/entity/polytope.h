// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/kernel.h"
#include "utility/generator.h"
#include "utility/lazy.h"
#include "utility/noncopyable.h"

#include <array>
#include <string>
#include <vector>

class Polytope final : public NonCopyable {
public:
    using VertexIndex = size_t;
    using FacetIndex = size_t;
    using FacetTopology = std::array<VertexIndex, DIMS>;
    using Face = std::vector<Point>;

    static Polytope loadObj(std::string filename);
    static Polytope invert(const Polytope& polytope);

    Polytope(
            std::string name,
            std::vector<Point> vertices,
            std::vector<FacetTopology> facetTopologies);

    const std::string& name() const
    {
        return name_;
    }
    const std::vector<Point>& vertices() const
    {
        return vertices_;
    }
    const std::vector<FacetTopology>& facetTopologies() const
    {
        return facetTopologies_;
    }

    const Facet& facetGeometry(FacetIndex facetIndex) const
    {
        return facetGeometries_[facetIndex];
    }
    Generator<const Facet&> facetGeometries() const;

    // Generate all faces of specified dimension
    // i.e having dim + 1 vertices
    Generator<const Face&> faces(size_t dim) const;

    Generator<const FacetIndex> neighborFacetIndices(
            FacetIndex facetIndex) const;

private:
    const std::string name_;
    const std::vector<Point> vertices_;
    const std::vector<FacetTopology> facetTopologies_;

    // Max-dimensional faces
    const std::vector<Facet> facetGeometries_;

    // Facet index -> vector of neighbor facet indices
    Lazy<std::vector<std::vector<FacetIndex>>> lazyNeighborsMap_;
};
