// This file is part of xscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "utility/noncopyable.h"

#include <array>
#include <chrono>
#include <ostream>

// RAII-style profiling helper
class Stopwatch final : public NonCopyable {
public:
    static constexpr size_t MAX_MEASUREMENTS = 10;
    static constexpr size_t MAX_NAME_LENGTH = 20;

    class Measurement final {
    public:
        Measurement()
        {
            name_.fill(0);
        }

        const char* name() const
        {
            return name_.data();
        }
        size_t callCount() const
        {
            return callCount_;
        }
        double runtimeSeconds() const
        {
            return runtimeSeconds_;
        }

    private:
        friend class Stopwatch;

        // Add trailing null to store valid c-strings
        std::array<char, MAX_NAME_LENGTH + 1> name_;
        size_t callCount_;
        double runtimeSeconds_;
    };

    static const Measurement& measurement(size_t index)
    {
        return measurements_.at(index);
    }

    static void dump(std::ostream& out);

    Stopwatch(size_t measurementIndex, const char* name);
    ~Stopwatch();

private:
    using Clock = std::chrono::high_resolution_clock;

    static std::array<Measurement, MAX_MEASUREMENTS> measurements_;

    const size_t measurementIndex_;
    Clock::time_point startTime_;
};

