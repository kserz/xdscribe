// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "geometry/entity/placement.h"
#include "geometry/entity/polytope.h"
#include "geometry/kernel.h"
#include "geometry/location/location.h"
#include "helper/io.h"
#include "helper/stats.h"
#include "helper/stopwatch.h"
#include "inscriber_factory.h"
#include "solver/inscriber.h"

#include <iostream>
#include <exception>

auto extractStopPredicate(const char* arg)
{
    if (arg[0] == 'v') {
        return Inscriber::StopPredicate(
            std::nullopt, std::atof(arg + 1), std::nullopt);
    } else if (arg[0] == 's') {
        return Inscriber::StopPredicate(
            std::nullopt, std::nullopt, std::chrono::seconds(std::atoi(arg + 1)));
    } else {
        return Inscriber::StopPredicate(
            std::atof(arg), std::nullopt, std::nullopt);
    }
}

int main(int argc, char** argv)
{
    if (argc < 5) {
        std::cout << "\nUsage: " << argv[0]
                  << " contour_file pattern_file stop_predicate inscriber_code "
                     "[ tries [ direct_magic ] ]" << std::endl;
        std::cout << "\nContour and pattern should be Wavefront .OBJ files "
                     "consisting of triangles only.\n";
        std::cout << "\nStop predicate is one of the following:\n"
                  << "\t<target precision>\n"
                  << "\tv<target value>\n"
                  << "\ts<seconds to run>\n";
        std::cout << "\nInscriber code is parsed according "
                     "to the following scheme.\n";
        dumpInscriberDescription(std::cout);
        std::cout << "\nTries specifies the number of algorithm runs within "
                  << "the single binary execution.\n";
        std::cout << "\ndirect_magic is the magic epsilon used by the DIRECT "
                     "algorithm of the NLopt library.\n";
        std::cout << std::endl;
        return -1;
    }

    try {
        auto pattern = Polytope::loadObj(argv[2]);
        auto contour = Polytope::loadObj(argv[1]);

        if (locatePoint(Point::constant(0), pattern.facetGeometries()) !=
                Location::Inner) {
            throw std::runtime_error(
                "coordinates origin must be inside the pattern");
        }

        const auto stopPredicate = extractStopPredicate(argv[3]);
        const unsigned tries = argc > 5 ? std::atoi(argv[5]) : 1;
        std::optional<double> magic;
        if (argc > 6) {
            magic = std::atof(argv[6]);
        }

        auto inscriber = makeInscriber(argv[4], magic);

        Placement result{Point::constant(0.), 0.};
        std::cout << "Running " << tries << " tries" << std::endl;
        {
            Stopwatch s(0, "overall");
            for (unsigned t = 0; t < tries; ++t) {
                result = (*inscriber)(pattern, contour, stopPredicate);
            }
        }

        std::cout.precision(15);
        std::cout << "Steps: " << Stats::instance().inscriberSteps;
        std::cout << "\nGeometry elements: "
                  << Stats::instance().geometryElementsCount.max();
        std::cout << "\nMax state size: " << Stats::instance().gridSize.max();
        std::cout << "\nAverage state size: "
                  << Stats::instance().gridSize.average();
        std::cout << "\nAverage sampling size: "
                  << Stats::instance().samplingSize.average();
        std::cout << "\nAverage sampling to grid ratio: "
                  << Stats::instance().samplingToGridRatio.average();
        for (auto kv : Stats::instance().polytopeFacesCount) {
            std::cout << "\n" << kv.first.name() << " faces " << kv.first.dims()
                      << " count " << kv.second;
        }
        std::cout << "\nPattern parts: "
                  << Stats::instance().patternConvexPartsCount;

        std::cout << std::endl;
        std::cout.precision(5);
        Stopwatch::dump(std::cout);

        std::cout << "\nImage types";
        std::cout.precision(5);
        for (size_t i = 0; i < Stats::ImageType::LAST; ++i) {
            std::cout << "\n  " <<  Stats::ImageType(i) << " : "
                      << Stats::instance().images.percentage(
                             Stats::ImageType(i));
        }

        std::cout << "\nObjective calls: " << Stats::instance().objectiveCalls;

        std::cout << "\n\nResult center: " << result.center()
                  << "\nResult radius: " << result.radius();

        std::cout << "\n\nRuntime of a single try: "
                  << Stopwatch::measurement(0).runtimeSeconds() / tries;

        std::cout << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "Unknown error" << std::endl;
    }

    return 0;
}
