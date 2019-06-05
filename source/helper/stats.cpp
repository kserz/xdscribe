// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#include "stats.h"

#include <map>
#include <string>

std::ostream &operator <<(std::ostream &out, ImageStats::ImageType imageType)
{
    using ImageType = ImageStats::ImageType;
    static const std::map<ImageType, std::string> typeNames = {
        {ImageType::Undefined, "undefined"},
        {ImageType::MostlyBoundary, "mostly boundary"},
        {ImageType::MostlyEmpty, "mostly empty"},
        {ImageType::MostlyFilled, "mostly filled"},
        {ImageType::FullyBoundary, "fully boundary"},
        {ImageType::FullyEmpty, "fully empty"},
        {ImageType::FullyFilled, "fully filled"},
    };

    out << typeNames.at(imageType);
    return out;
}
