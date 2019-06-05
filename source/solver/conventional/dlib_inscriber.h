// This file is part of xscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "solver/inscriber.h"

class DlibInscriber final : public Inscriber
{
public:
    virtual Placement operator ()(
            const Polytope& pattern,
            const Polytope& contour,
            const StopPredicate& stopPredicate) const override;
};
