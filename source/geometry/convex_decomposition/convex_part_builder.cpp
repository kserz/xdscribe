// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "convex_part_builder.h"

#include "geometry/utility/cgal.h"
#include "geometry/entity/perpendicular.h"
#include "geometry/location/simplex_facet_overlap.h"

#include <iterator>
#include <optional>

namespace {

using Triangulation = CGAL::Triangulation<Kernel>;
using VertexIndex = Polytope::VertexIndex;
using FacetIndex = Polytope::FacetIndex;

struct LocationInfo {
    LocationInfo()
        : f(DIMS)
    {}

    Triangulation::Full_cell_handle cellHandle;
    Triangulation::Locate_type location;
    Triangulation::Face f;
    Triangulation::Facet ft;
};

LocationInfo locate(
        const Triangulation& triangulation,
        const Kernel::Point_d& point)
{
    LocationInfo result;
    result.cellHandle = triangulation.locate(
        point, result.location, result.f, result.ft);
    return result;
}

// We assume star-shaped polytopes to be centered at 0
Vector<> outsideNormal(const Facet& facet)
{
    auto result = unitNormal(facet);
    if (result.dot(facet[0]) < 0.) {
        result *= -1.;
    }
    return result;
}

template<class Points>
Point centroid(const Points& points)
{
    auto result = Point::constant(0.);
    auto pointsNumber = static_cast<double>(points.size());
    for (const auto& point : points)
    {
        result += point / pointsNumber;
    }
    return result;
}

Point outerPoint(const Facet& facet)
{
    // The point with a small offset still could land in a wrong cell
    // but in real life it don't
    return (centroid(facet) + outsideNormal(facet) * 2. * MEPS).eval();
}

Triangulation::Full_cell_handle findOuterCell(
        const Triangulation& triangulation,
        const Facet& facet)
{
    const auto locationInfo = locate(triangulation, toCGAL(outerPoint(facet)));
    assert(locationInfo.location ==
        Triangulation::Locate_type::OUTSIDE_CONVEX_HULL);
    return locationInfo.cellHandle;
}

std::optional<VertexIndex> findNewVertex(
        const std::set<VertexIndex>& usedVertices,
        const Polytope::FacetTopology& newFacetTopology)
{
    std::optional<VertexIndex> result;
    for (auto&& vertexIndex : newFacetTopology) {
        if (usedVertices.count(vertexIndex) == 0) {
            // We check that only one vertex is new
            assert(result == std::nullopt);
            result = vertexIndex;
        }
    }
    return result;
}

// Gather all the cells to be modified by vertex insertion
//     i.e. infinite ones having facets visible from point
std::vector<Triangulation::Full_cell_handle> collectAffectedCells(
        Triangulation* triangulation,
        const Kernel::Point_d& point,
        Triangulation::Full_cell_handle pointCell)
{
    std::vector<Triangulation::Full_cell_handle> result;
    result.reserve(64);
    auto ori = triangulation->geom_traits().orientation_d_object();
    Triangulation::Outside_convex_hull_traversal_predicate<decltype(ori)>
        ochtp(*triangulation, point, ori);
    auto out = std::back_inserter(result);
    triangulation->tds().gather_full_cells(pointCell, ochtp, out);
    return result;
}

bool isSimplexInsidePolytope(
        const Simplex& simplex,
        const Polytope& polytope)
{
    SimplexFacetOverlap simplexOverlap(&simplex);
    bool crossesBoundary = false;
    polytope.facetGeometries().process([&] (auto&& facet) {
        if (simplexOverlap(facet) == Location::Inner) {
            crossesBoundary = true;
        }
    });
    if (crossesBoundary) {
        return false;
    }

    if (locatePoint(centroid(simplex), polytope.facetGeometries()) ==
            Location::Outer) {
        return false;
    }
    return true;
}

// Simplex part of the infinite cell bounded by the point inside
Simplex boundedPart(
        const Triangulation& triangulation,
        Triangulation::Full_cell_handle infiniteCell,
        const Triangulation::Point& pointInside)
{
    Simplex result;
    for (size_t i = 0; i < result.size(); ++i) {
        const auto cellVertexHandle = infiniteCell->vertex(i);
        if (triangulation.is_infinite(cellVertexHandle)) {
            result[i] = fromCGAL(pointInside);
        } else {
            result[i] = fromCGAL(cellVertexHandle->point());
        }
    }
    return result;
}

} // namespace

ConvexPartBuilder::ConvexPartBuilder(
        const Polytope* starShapedPolytope,
        Polytope::FacetIndex startFacetIndex)
    : polytope_(starShapedPolytope)
    , triangulation_(DIMS)
{
    triangulation_.insert(toCGAL(Point::constant(0)));
    for (auto&& vertexIndex :
            polytope_->facetTopologies()[startFacetIndex]) {
        usedVertices_.insert(vertexIndex);
        triangulation_.insert(toCGAL(polytope_->vertices()[vertexIndex]));
    }
    assert(triangulation_.current_dimension() == DIMS);
    occupiedInfiniteCells_.emplace(
        findOuterCell(
            triangulation_,
            polytope_->facetGeometry(startFacetIndex)),
        std::move(startFacetIndex));
}

bool ConvexPartBuilder::tryAddFacet(Polytope::FacetIndex facetIndex)
{
    const auto newVertexIndex = findNewVertex(
        usedVertices_, polytope_->facetTopologies()[facetIndex]);
    if (!newVertexIndex) {
        occupiedInfiniteCells_.emplace(
            findOuterCell(
                triangulation_,
                polytope_->facetGeometry(facetIndex)),
            std::move(facetIndex));
        return true;
    }

    const auto newPoint = toCGAL(polytope_->vertices()[*newVertexIndex]);
    const auto locationInfo = locate(triangulation_, newPoint);

    if (locationInfo.location == Triangulation::Locate_type::IN_FULL_CELL) {
        return false;
    }
    assert(locationInfo.location ==
        Triangulation::Locate_type::OUTSIDE_CONVEX_HULL);
    const auto targetCellHandle = locationInfo.cellHandle;

    // NB: targetCellHandle is included in affectedCells
    const auto affectedCells =
        collectAffectedCells(&triangulation_, newPoint, targetCellHandle);
    for (const auto& affectedCell : affectedCells) {
        assert(triangulation_.is_infinite(affectedCell));

        // New point should not cover any facet of the current convex hull
        if (occupiedInfiniteCells_.count(affectedCell)) {
            return false;
        }
        // On point insertion all the affected cells are replaced by their
        // bounded parts. So we check them to be actually in the polytope.
        if (!isSimplexInsidePolytope(
                boundedPart(triangulation_, affectedCell, newPoint),
                *polytope_)) {
            return false;
        }
    }

    triangulation_.insert_in_hole(
        newPoint,
        affectedCells.begin(),
        affectedCells.end(),
        Triangulation::Facet(
            targetCellHandle,
            targetCellHandle->index(triangulation_.infinite_vertex())));
    usedVertices_.insert(*newVertexIndex);
    occupiedInfiniteCells_.emplace(
        findOuterCell(
            triangulation_,
            polytope_->facetGeometry(facetIndex)),
        std::move(facetIndex));
    return true;
}

std::vector<Point> ConvexPartBuilder::vertices() const
{
    std::vector<Point> result;
    result.reserve(usedVertices_.size() + 1);
    result.push_back(Point::constant(0.));
    for (auto&& vertexIndex : usedVertices_) {
        result.push_back(polytope_->vertices()[vertexIndex]);
    }
    return result;
}
