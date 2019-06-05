// This file is part of xscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "solver/conventional/objective_bounder.h"

ValueBounds computeCombinedBounds(
        const Objective& objective,
        const Region& region);
