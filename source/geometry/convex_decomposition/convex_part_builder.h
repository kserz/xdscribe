// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "geometry/entity/polytope.h"
#include "geometry/kernel.h"
#include "geometry/utility/cgal.h"
#include "utility/noncopyable.h"

#include <CGAL/Triangulation.h>

#include <set>
#include <map>

class ConvexPartBuilder final : public NonCopyable {
public:
   ConvexPartBuilder(
            const Polytope* starShapedPolytope,
            Polytope::FacetIndex startFacetIndex);

   // Returns true if the facet have been actually added
   bool tryAddFacet(Polytope::FacetIndex facetIndex);

   std::vector<Point> vertices() const;

private:
   using Triangulation = CGAL::Triangulation<Kernel>;

   const Polytope* const polytope_;

   std::set<Polytope::VertexIndex> usedVertices_;
   Triangulation triangulation_;
   // Infinite cells bounded by already added facets
   std::map<Triangulation::Full_cell_handle, Polytope::FacetIndex>
       occupiedInfiniteCells_;
};
