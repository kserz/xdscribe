// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "inscriber_factory.h"

#include "geometry/convex_decomposition/convex_decomposition.h"
#include "geometry/convex_decomposition/floodfill_convex_decomposition.h"
#include "geometry/convex_decomposition/star_convex_decomposition.h"
#include "grid/rasterization/bbox_facet_rasterizer.h"
#include "grid/rasterization/facet_rasterizer.h"
#include "grid/rasterization/inner_region_rasterizer.h"
#include "grid/rasterization/polytope_rasterizer.h"
#include "grid/sampling/vector_sparse_raster.h"
#include "solver/conventional/combined_objective_bounder.h"
#include "solver/conventional/dlib_inscriber.h"
#include "solver/conventional/ghj_inscriber.h"
#include "solver/conventional/nlopt_inscriber.h"
#include "solver/conventional/objective_bounder.h"
#include "solver/inverse/graphic_inscriber.h"
#include "solver/inverse/halfspace_part_rasterizer.h"
#include "solver/inverse/minkowski_sum_rasterizer.h"

#include <cstddef>
#include <functional>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

// --- General factory definition

using Text = std::vector<std::string>;

template<typename... TextParts>
Text merge(TextParts&&... parts)
{
    Text result;
    result.reserve((... + parts.size()));
    auto addPart = [&] (auto&& part) {
        for (auto& line : part) {
            result.push_back(std::move(line));
        }
    };
    (... , addPart(std::move(parts)));
    return result;
}

// No thread-safety!
class StructureLogger final {
public:
    StructureLogger(const std::string& levelName)
    {
        std::cout << makeOffset(level_++) << levelName << std::endl;
    }

    ~StructureLogger()
    {
        --level_;
    }

    static Text addLevel(std::string levelName, Text contents)
    {
        for (auto& line : contents) {
            line = makeOffset(1) + std::move(line);
        }
        contents.insert(contents.begin(), std::move(levelName));
        return std::move(contents);
    }

private:
    static std::string makeOffset(size_t level) {
        return std::string(level * 4, ' ');
    }

    static size_t level_;
};

size_t StructureLogger::level_ = 0;

template<typename... Params>
struct ParametrizedBase final {
    template<typename Result>
    struct Factory {
        Result operator ()(Params... params) const
        {
            return value(params...);
        }

        const Text description;
        const std::function<Result(Params...)> value;
    };

    template<typename Result>
    static Factory<Result> valueFactory(
            std::string description,
            Result value)
    {
        return {
            {description},
            [description = std::move(description), value = std::move(value)]
                    (Params...) -> Result {
                StructureLogger log(description);
                return value;
            }
        };
    }

    template<typename Result, typename... Args>
    static Factory<Result> composition(
            std::string description,
            std::function<Result(Args..., Params...)> f,
            Factory<Args>... args)
    {
        return {
            StructureLogger::addLevel(
                description,
                merge(std::move(args.description)...)),
            [
                description = std::move(description),
                f = std::move(f),
                args...
            ] (Params... params) -> Result {
                StructureLogger log(description);
                // We need to call the args in the right order
                return std::apply(
                    f, std::tuple<Args..., Params...>{
                        args(params...)..., params...});
            }
        };
    }
};

class CodeReader final {
public:
    CodeReader(const char* code)
        : code_(code)
    {}

    char next()
    {
        return *code_++;
    }

private:
    const char* code_;
};

using NloptMagic = std::optional<double>;

using Parametrized = ParametrizedBase<CodeReader*, NloptMagic>;

template<typename Result>
Parametrized::Factory<Result> selectionFactory(
        std::string description,
        std::map<char, Parametrized::Factory<Result>> selection)
{
    Text choicesDescription;
    for (auto& [code, factory] : selection) {
        assert(!factory.description.empty());
        choicesDescription.push_back(
            std::string{code} + " - " + std::move(factory.description[0]));
        choicesDescription.insert(
            choicesDescription.end(),
            std::make_move_iterator(++factory.description.begin()),
            std::make_move_iterator(factory.description.end()));
    }

    return {
        StructureLogger::addLevel(
            description + ":",
            std::move(choicesDescription)),
        [description = std::move(description), selection = std::move(selection)]
                (CodeReader* codeReader, auto... params) -> Result {
            const auto code = codeReader->next();
            if (!selection.count(code)) {
                throw std::runtime_error(
                    std::string{"unknown "} +
                    description + std::string{" code "} +
                    (code ? std::string{code} : std::string{"null"}));
            }
            return selection.at(code)(codeReader, params...);
        }
    };
}

// --- Inscriber factory definition

const auto convexDecompositorFactory = [] (const auto description) {
    return selectionFactory<ConvexDecompositor>(
        std::move(description),
        {
            {'c', Parametrized::valueFactory<ConvexDecompositor>(
                "dummy convex decompositor", dummyDecomposition)},
            {'f', Parametrized::valueFactory<ConvexDecompositor>(
                "flood-fill convex decompositor", floodFillDecomposition)},
            {'s', Parametrized::valueFactory<ConvexDecompositor>(
                "star convex decompositor", starDecomposition)}
        });
};

const auto facetRasterizerFactory =
    selectionFactory<FacetRasterizer>(
        "facet rasterizer",
        {
            {'b', Parametrized::valueFactory<FacetRasterizer>(
                "bbox facet rasterizer", bBoxFacetRasterizer())},
            {'o', Parametrized::valueFactory<FacetRasterizer>(
                "overlap facet rasterizer", rasterizeFacetByOverlap)}
        });

const auto innerRegionRasterizerFactory =
    selectionFactory<InnerRegionRasterizer>(
        "inner rasterizer",
        {
            {'f', Parametrized::valueFactory<InnerRegionRasterizer>(
                 "facets-combined inner rasterizer",
                 rasterizeInnerRegionByFacets)},
            {'r', Parametrized::valueFactory<InnerRegionRasterizer>(
                 "ray-combined inner rasterizer",
                 rasterizeInnerRegionByRays)},
            {'s', Parametrized::valueFactory<InnerRegionRasterizer>(
                 "sequental inner rasterizer",
                 rasterizeInnerRegionSequentally)}
        });

const auto polytopeRasterizerFactory = [] (const auto description) {
    return Parametrized::composition<
            PolytopeRasterizer, FacetRasterizer, InnerRegionRasterizer>(
        description,
        {[] (
                FacetRasterizer facetRasterizer,
                InnerRegionRasterizer innerRegionRasterizer,
                auto...) {
            return polytopeRasterizer(
                std::move(facetRasterizer),
                std::move(innerRegionRasterizer));
        }},
        facetRasterizerFactory,
        innerRegionRasterizerFactory);
};

const auto minkowskiSumRasterizerFactory =
    selectionFactory<MinkowskiSumRasterizer>(
        "minkowski sum rasterizer",
        {
            {'h', Parametrized::valueFactory<MinkowskiSumRasterizer>(
                "halfspaces minkowski sum rasterizer",
                decomposingMSRasterizer<VectorSampling>(
                    rasterizePartByHalfspaces))},
            {'p', Parametrized::composition<
                MinkowskiSumRasterizer, PolytopeRasterizer>(
                    "polytope-based minkowski sum rasterizer",
                    {[] (PolytopeRasterizer polytopeRasterizer, auto...) {
                        return decomposingMSRasterizer<VectorSampling>(
                            polytopePartRasterizer(
                                std::move(polytopeRasterizer)));
                    }},
                    polytopeRasterizerFactory("convex part polytope rasterizer"))}
        });

const auto graphicInscriberFactory = Parametrized::composition<
    std::unique_ptr<Inscriber>,
    ConvexDecompositor,
    MinkowskiSumRasterizer,
    PolytopeRasterizer>(
        "graphic inverse inscriber",
        {[] (
                ConvexDecompositor convexDecompositor,
                MinkowskiSumRasterizer minkowskiSumRasterizer,
                PolytopeRasterizer polytopeRasterizer,
                auto...) {
            return std::make_unique<GraphicInscriber>(
                std::move(convexDecompositor),
                std::move(minkowskiSumRasterizer),
                std::move(polytopeRasterizer));
        }},
        convexDecompositorFactory("pattern convex decompositor"),
        minkowskiSumRasterizerFactory,
        polytopeRasterizerFactory("contour polytope rasterizer"));

const auto objectiveBounderFactory =
    selectionFactory<ObjectiveBounder>(
        "objective bounder",
        {
            {'s', Parametrized::valueFactory<ObjectiveBounder>(
                "simple objective bounder", computeSimpleBounds)},
            {'c', Parametrized::valueFactory<ObjectiveBounder>(
                "combined objective bounder", computeCombinedBounds)},
        });

const auto ghjInscriberFactory = Parametrized::composition<
    std::unique_ptr<Inscriber>,
    ObjectiveBounder>(
        "GHJ inscriber",
        {[] (ObjectiveBounder objectiveBounder, auto...) {
            return std::make_unique<GHJInscriber>(std::move(objectiveBounder));
        }},
        objectiveBounderFactory);

const auto nloptAlgorithmFactory =
    selectionFactory<NloptInscriber::Algorithm>(
        "nlop algorithm",
        {
            {'n', Parametrized::valueFactory<NloptInscriber::Algorithm>(
                "DIRECT non-scaled nlop algorithm",
                NloptInscriber::Algorithm::DirectNonScaled)},
            {'s', Parametrized::valueFactory<NloptInscriber::Algorithm>(
                "DIRECT scaled nlop algorithm",
                NloptInscriber::Algorithm::DirectScaled)},
        });

const auto nloptInscriberFactory = Parametrized::composition<
    std::unique_ptr<Inscriber>,
    NloptInscriber::Algorithm>(
        "NLopt inscriber",
        {[] (NloptInscriber::Algorithm algorithm, auto, NloptMagic nloptMagic) {
            return std::make_unique<NloptInscriber>(algorithm, nloptMagic);
        }},
        nloptAlgorithmFactory);

const Parametrized::Factory<std::unique_ptr<Inscriber>> dlibInscriberFactory{
    {"dlib inscriber"},
    [] (auto...) {
        return std::make_unique<DlibInscriber>();
    }
};

const auto inscriberFactory =
    selectionFactory<std::unique_ptr<Inscriber>>(
        "inscriber",
        {
            {'d', dlibInscriberFactory},
            {'g', graphicInscriberFactory},
            {'h', ghjInscriberFactory},
            {'n', nloptInscriberFactory}
        });

} // namespace

std::unique_ptr<Inscriber> makeInscriber(
        const char* codeString,
        std::optional<double> nloptMagic)
{
    assert(codeString != nullptr);
    std::cout << "Creating ";
    CodeReader codeReader(codeString);
    return inscriberFactory(&codeReader, nloptMagic);
}

void dumpInscriberDescription(std::ostream& out)
{
    for (const auto& line : inscriberFactory.description) {
        out << line << std::endl;
    }
}
