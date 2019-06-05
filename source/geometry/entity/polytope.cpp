// This file is part of xscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "polytope.h"

#include "helper/stats.h"
#include "utility/subsets.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <set>

using VertexIndex = Polytope::VertexIndex;
using FacetIndex = Polytope::FacetIndex;
using FacetTopology = Polytope::FacetTopology;
// Stores sorted indices of vertices
using FaceTopology = std::vector<Polytope::VertexIndex>;

namespace {

std::vector<Facet> extractFacetGeometries(
        const std::vector<FacetTopology>& facetTopologies,
        const std::vector<Point>& vertices)
{
    std::vector<Facet> result;
    result.reserve(facetTopologies.size());
    for (const auto& facetTopology : facetTopologies) {
        Facet facetGeometry;
        for (size_t i = 0; i < facetGeometry.size(); ++i) {
            facetGeometry[i] = vertices[facetTopology[i]];
        }
        result.push_back(std::move(facetGeometry));
    }
    return result;
}

std::vector<std::vector<FacetIndex>> evalNeighborsMap(
        const std::vector<FacetTopology>& facetTopolgies)
{
    assert(DIMS >= 0);

    std::multimap<FaceTopology, FacetIndex> edgesToFacetIndices;

    for (FacetIndex facetIndex = 0;
            facetIndex < facetTopolgies.size();
            ++facetIndex) {
        subSets<VertexIndex>(&facetTopolgies[facetIndex], DIMS - 1).process(
                [&] (auto&& face) {
            std::sort(face.begin(), face.end());
            edgesToFacetIndices.emplace(std::move(face), facetIndex);
        });
    }

    std::vector<std::vector<FacetIndex>> neighborsMap(facetTopolgies.size());
    for (auto& neighbors : neighborsMap) {
        neighbors.reserve(DIMS);
    }

    for (auto it = edgesToFacetIndices.begin();
            it != edgesToFacetIndices.end();) {
        auto neighborIt = std::next(it);

        // Every edge should have exactly 2 incident facets in polytope
        assert(neighborIt != edgesToFacetIndices.end() &&
                neighborIt->first == it->first);
        auto subsequent = std::next(neighborIt);
        if (subsequent != edgesToFacetIndices.end()) {
            assert(subsequent->first != it->first);
        }

        neighborsMap[it->second].push_back(neighborIt->second);
        neighborsMap[neighborIt->second].push_back(it->second);

        it = subsequent;
    }

    for (const auto& neighbors : neighborsMap) {
        assert(neighbors.size() == DIMS);
    }

    return neighborsMap;
}

} // namespace

Polytope Polytope::loadObj(std::string filename)
{
    std::ifstream file(filename);
    if (!file)
        throw std::runtime_error("Error accessing file!");

    std::vector<Point> vertices;
    std::vector<FacetTopology> facets;

    char c;
    Point point;
    FacetTopology facet;

    while (file.get(c)) {
        switch (c) {
        case 'v':
            for (auto& coord : point) {
                file >> coord;
            }
            vertices.push_back(point);
            break;
        case 'f':
            for (auto& vertexIdx : facet) {
                file >> vertexIdx;
                --vertexIdx;
            }
            facets.push_back(facet);
            break;
        case '\n':
            continue;
        default:
            break;
        }
        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    return {filename, vertices, facets};
}

Polytope Polytope::invert(const Polytope& polytope)
{
    std::vector<Point> invertedVertices(polytope.vertices());
    for (auto& vertex : invertedVertices) {
        vertex *= -1.;
    }
    return {
        polytope.name(),
        std::move(invertedVertices),
        polytope.facetTopologies()
    };
}

Polytope::Polytope(
        std::string name,
        std::vector<Point> vertices,
        std::vector<FacetTopology> facetTopologies)
    : name_(std::move(name))
    , vertices_(std::move(vertices))
    , facetTopologies_(std::move(facetTopologies))
    , facetGeometries_(extractFacetGeometries(facetTopologies_, vertices_))
    , lazyNeighborsMap_([this] {
        return evalNeighborsMap(facetTopologies_);
    })
{
    Stats::PolytopeFaceKey facetsKey {name_, DIMS - 1};
    Stats::instance().polytopeFacesCount[facetsKey] = facetTopologies_.size();
}

Generator<const Facet&> Polytope::facetGeometries() const
{
    return Generator<const Facet&>(&facetGeometries_);
}

Generator<const Polytope::Face&> Polytope::faces(size_t dim) const
{
    assert(dim < DIMS);
    return Generator<const Face&>([this, dim] (auto&& yield) {
        std::set<FaceTopology> uniqueFaces;

        for (const auto& facetTopology : facetTopologies_) {
            subSets<VertexIndex>(&facetTopology, dim + 1).process(
                    [&] (auto&& face) {
                std::sort(face.begin(), face.end());
                uniqueFaces.insert(std::move(face));
            });
        }

        for (const auto& indexFace : uniqueFaces) {
            Face geometry;
            geometry.reserve(indexFace.size());
            for (const auto vertexIndex : indexFace) {
                geometry.push_back(vertices_[vertexIndex]);
            }
            assert(geometry.size() == dim + 1);
            yield(geometry);
        }

        Stats::PolytopeFaceKey facetsKey {name_, dim};
        Stats::instance().polytopeFacesCount[facetsKey] = uniqueFaces.size();
    });
}

Generator<const FacetIndex> Polytope::neighborFacetIndices(
        FacetIndex facetIndex) const
{
    return Generator<const FacetIndex>(&lazyNeighborsMap_()[facetIndex]);
}
