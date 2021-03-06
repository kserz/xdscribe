// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "solver/inscriber.h"

#include <memory>
#include <optional>
#include <ostream>

// codeString should be null-terminated
std::unique_ptr<Inscriber> makeInscriber(
        const char* codeString,
        std::optional<double> nloptMagic = std::nullopt);

void dumpInscriberDescription(std::ostream& out);
