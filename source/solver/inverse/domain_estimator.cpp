#include "domain_estimator.h"

#include "grid/sampling/vector_sparse_raster.h"

DomainEstimatorFactory graphicDomainEstimatorFactory(
        MinkowskiSumRasterizer minkowskiSumRasterizer,
        PolytopeRasterizer contourRasterizer)
{
    return [
            msumRasterizer = std::move(minkowskiSumRasterizer),
            contourRasterizer = std::move(contourRasterizer)] (
            const MinkowskiSum* minkowskiSum,
            const Polytope* contour) -> DomainEstimator
    {
        return [
                msumRasterizer,
                contourRasterizer,
                minkowskiSum,
                contour] (
                const Sampling<Location>& sampling, double radius) {
            VectorSampling<Location> result{
                sampling,
                VectorSparseRaster{
                    mapGenerator<const Coordinates&>(
                        sampling.voxels(),
                        [] (const auto& voxel) {
                            return voxel.coordinates();
                        }),
                    Location::Outer,
                    sampling.size()
                }
            };
            VectorSparseRaster<Location> feasibility = result;

            msumRasterizer(*minkowskiSum, radius, &result);
            contourRasterizer(
                        sampling.toLocal(contour->facetGeometries()),
                        &feasibility);

            result.voxels().process([&] (auto& voxel) {
                auto* feasibilityVoxel = feasibility.find(voxel.coordinates());
                assert(feasibilityVoxel);

                // Inner voxels of the image do not contain a good solution,
                // thus they are empty for a problem.
                if (voxel.value == Location::Inner ||
                        feasibilityVoxel->value == Location::Outer) {
                    voxel.value = Location::Outer;
                } else if (voxel.value == Location::Outer) {
                    voxel.value = Location::Inner;
                }
            });

            return result;
        };
    };
}
