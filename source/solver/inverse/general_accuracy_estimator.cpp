#include "general_accuracy_estimator.h"

#include "geometry/kernel.h"
#include "grid/sampling/refinement.h"
#include "grid/sampling/vector_sampling.h"
#include "grid/sampling/vector_sparse_raster.h"

AccuracyEstimatorFactory generalAccuracyEstimatorFactory()
{
    return [] (
            const DomainEstimator* domainEstimator,
            const Polytope* /*starShapedPattern*/,
            double targetPrecision) -> ActualAccuracyEstimator
    {
        return [domainEstimator, targetPrecision] (
                double radius,
                double radiusAccuracy,
                const Sampling<Location>& sampling) -> double {
            double result = radiusAccuracy;
            VectorSampling<Location> accuracySampling{
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

            while (result < targetPrecision + MEPS) {
                const auto newSampling = (*domainEstimator)(
                    accuracySampling, radius + result);

                bool haveNonEmptyVoxels = false;
                newSampling.voxels().process([&] (const auto& voxel) {
                    if (voxel.value != Location::Outer) {
                        haveNonEmptyVoxels = true;
                    }
                });

                if (haveNonEmptyVoxels) {
                    result += radiusAccuracy;
                    accuracySampling = shrink(newSampling);
                } else {
                    break;
                }
            }

            return result;
        };
    };
}
