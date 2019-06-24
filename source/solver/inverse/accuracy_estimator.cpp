#include "accuracy_estimator.h"

#include "geometry/kernel.h"
#include "solver/inscribed_radius.h"

AccuracyEstimatorFactory lipschitzianAccuracyEstimatorFactory()
{
    return [] (
            const DomainEstimator* /*domainEstimator*/,
            const Polytope* starShapedPattern,
            double /*targetPrecision*/) -> ActualAccuracyEstimator {
        auto gridLipschitzConstant =
            InscribedRadius::lipschitzConstant(*starShapedPattern) * sqrt(DIMS);
        return [gridLipschitzConstant] (
                double /*radius*/,
                double radiusAccuracy,
                const Sampling<Location>& sampling) {
            return radiusAccuracy + gridLipschitzConstant * sampling.gridStep();
        };
    };
}
