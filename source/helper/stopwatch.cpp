// This file is part of xscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "stopwatch.h"

#include <cassert>
#include <cstring>

std::array<Stopwatch::Measurement, Stopwatch::MAX_MEASUREMENTS>
    Stopwatch::measurements_;

Stopwatch::Stopwatch(size_t measurementIndex, const char *name)
    : measurementIndex_(measurementIndex)
{
    auto* currentName = measurements_.at(measurementIndex).name_.data();
    if (currentName[0] != 0) {
        assert(!std::strncmp(currentName, name, MAX_NAME_LENGTH));
    } else {
        std::strncpy(currentName, name, MAX_NAME_LENGTH);
    }

    startTime_ = Clock::now();
}

Stopwatch::~Stopwatch()
{
    auto endTime = Clock::now();
    double durationSeconds = static_cast<double>(
        std::chrono::duration_cast<std::chrono::microseconds>(
            endTime - startTime_).count()) / 1e6;

    auto& currentMeasurement = measurements_.at(measurementIndex_);
    ++currentMeasurement.callCount_;
    currentMeasurement.runtimeSeconds_ += durationSeconds;
}

void Stopwatch::dump(std::ostream& out)
{
    out << "Stopwatch measurements: \n\n";
    for (size_t i = 0; i < Stopwatch::MAX_MEASUREMENTS; ++i) {
        const auto& measurement = Stopwatch::measurement(i);
        if (strlen(measurement.name()) == 0) {
            continue;
        }
        out << "  " << measurement.name() << " : "
                  << measurement.callCount() << " ~ "
                  << measurement.runtimeSeconds() << " s\n";
    }
}
