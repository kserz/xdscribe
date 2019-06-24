// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "solver/inscriber.h"

#include <nlopt.hpp>

#include <optional>

class NloptInscriber final : public Inscriber
{
public:
    enum class Algorithm {
        DirectScaled,
        DirectLocalScaled,
        DirectLocalRandomizedScaled,
        DirectNonScaled,
        DirectLocalNonScaled,
        DirectLocalRandomizedNonScaled
    };

    NloptInscriber(
            Algorithm algorithm,
            std::optional<double> magic = std::nullopt);

    virtual Placement operator ()(
            const Polytope& pattern,
            const Polytope& contour,
            const StopPredicate& stopPredicate) const override;

private:
    const Algorithm algorithm_;
    const std::optional<double> magic_;
};
