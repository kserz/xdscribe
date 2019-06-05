// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "io.h"

#include "geometry/utility/xd_iterator.h"

#include <fstream>
#include <map>
#include <optional>
#include <string>

namespace {

using VertexIndices = std::array<size_t, DIMS>;

class ObjBuilder {
public:
    void addFacet(const Facet& facet) {
        size_t offset = vertices_.size();
        for (const auto& vertex : facet) {
            vertices_.push_back(vertex);
        }
        VertexIndices vertexIndices;
        for (size_t i = 0; i < DIMS; ++i) {
            vertexIndices[i] = offset + i + 1;
        }
        faces_.push_back(std::move(vertexIndices));
    }

    void addTetrahedronAt(const Point& point) {
        static constexpr double scale = 0.1;
        static const std::vector<Point> modelVertices = {
            {0.000000, 1.000000, -1.502657},
            {0.000000, 0.000000, 0.497343},
            {0.866025, -0.500000, -1.502657},
            {-0.866025, -0.500000, -1.502657}
        };
        static const std::vector<VertexIndices> modelFaces = {
            {4, 2, 1},
            {1, 2, 3},
            {3, 2, 4},
            {1, 3, 4}
        };
        addModelAt(point, scale, modelVertices, modelFaces);
    }

    void addBoxAt(const Point& point, double scale = 1.0)
    {
        static const std::vector<Point> modelVertices = {
            {-0.500000, -0.500000, -0.500000},
            {-0.500000, -0.500000, 0.500000},
            {-0.500000, 0.500000, -0.500000},
            {-0.500000, 0.500000, 0.500000},
            {0.500000, -0.500000, -0.500000},
            {0.500000, -0.500000, 0.500000},
            {0.500000, 0.500000, -0.500000},
            {0.500000, 0.500000, 0.500000}
        };
        static const std::vector<VertexIndices> modelFaces = {
            {4, 1, 2},
            {8, 3, 4},
            {6, 7, 8},
            {2, 5, 6},
            {3, 5, 1},
            {8, 2, 6},
            {4, 3, 1},
            {8, 7, 3},
            {6, 5, 7},
            {2, 1, 5},
            {3, 7, 5},
            {8, 4, 2}
        };
        addModelAt(point, scale, modelVertices, modelFaces);
    }

    void write(const std::string& filename) const
    {
        std::ofstream file(filename);
        if (!file)
            throw std::runtime_error(
                std::string("Error accessing file ") + filename);

        for (const auto& vertex : vertices_) {
            file << 'v';
            for (const auto& coordinate : vertex) {
                file << ' ' << coordinate;
            }
            file << '\n';
        }

        for (const auto& face : faces_) {
            file << 'f';
            for (const auto& vertexIndex : face) {
                file << ' ' << vertexIndex;
            }
            file << '\n';
        }
    }

private:
    void addModelAt(
            const Point& point,
            double scale,
            const std::vector<Point>& vertices,
            const std::vector<VertexIndices>& faces)
    {
        size_t offset = vertices_.size();
        for (const auto& vertex : vertices) {
            vertices_.push_back((point + vertex * scale).eval());
        }
        for (auto face : faces) {
            for (auto& idx : face) {
                idx += offset;
            }
            faces_.push_back(std::move(face));
        }
    }

    std::vector<Point> vertices_;
    std::vector<VertexIndices> faces_;
};

} // namespace

std::ostream&operator <<(std::ostream& out, Location location)
{
    static const std::map<Location, std::string> locationNames = {
        {Location::Outer, "outer"},
        {Location::Boundary, "boundary"},
        {Location::Inner, "inner"},
    };
    out << locationNames.at(location);
    return out;
}

void drawPoints(const std::string& filename, const std::vector<Point>& points)
{
    ObjBuilder objBuilder;
    for (const auto& point : points) {
        objBuilder.addTetrahedronAt(point);
    }
    objBuilder.write(filename);
}

void drawFacets(
        const std::string& filename,
        const Generator<const Facet&>& facets)
{
    ObjBuilder objBuilder;
    facets.process([&] (const auto& facet) {
        objBuilder.addFacet(facet);
    });
    objBuilder.write(filename);
}

void drawSampling(
        const std::string& filename,
        const Sampling<Location>& sampling)
{
    static const std::map<Location, double> locationModelScales = {
        {Location::Outer, 0.05},
        {Location::Boundary, 0.6},
        {Location::Inner, 0.8}
    };

    ObjBuilder objBuilder;
    XDIterator<>::run(
        sampling.rasterSize(),
        [&] (const Coordinates& coordinates) {
            const auto it = sampling.find(coordinates);
            if (!it) {
                return;
            }

            objBuilder.addBoxAt(
                (coordinates.cast<double>() + Point::constant(0.5)).eval(),
                locationModelScales.at(it->value));
        });
    objBuilder.write(filename);
}

void dumpSampling(
        const std::string& filename,
        const Sampling<Location>& sampling)
{
    static const std::map<Location, char> locationSymbols = {
        {Location::Outer, '.'},
        {Location::Boundary, '+'},
        {Location::Inner, '#'}
    };
    static const char emptySymbol = '_';

    std::ofstream file(filename);
    if (!file) {
        throw std::runtime_error(
            std::string("Error accessing file ") + filename);
    }

    XDIterator<>::run(
        sampling.rasterSize(),
        [&] (const Coordinates& coordinates) {
            const auto it = sampling.find(coordinates);
            file << (it ? locationSymbols.at(it->value) : emptySymbol) << ' ';
            for (size_t i = 0; i < DIMS; ++i) {
                if (coordinates[i] == sampling.rasterSize()[i] - 1) {
                    file << '\n';
                } else {
                    break;
                }
            }
        });
}
