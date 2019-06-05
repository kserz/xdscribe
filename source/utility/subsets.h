// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include "utility/generator.h"

#include <algorithm>
#include <vector>

template<class Element, class Container>
Generator<std::vector<Element>> subSets(
        const Container* superSet,
        size_t subSetSize)
{
    using SubSet = std::vector<Element>;
    return Generator<SubSet>([superSet, subSetSize] (auto&& yield) {
        std::vector<bool> selectionMask(superSet->size());
        for (size_t i = 0; i < selectionMask.size(); ++i) {
            selectionMask[i] = i < subSetSize;
        }

        do {
            SubSet subSet;
            subSet.reserve(subSetSize);

            for (size_t i = 0; i < selectionMask.size(); ++i) {
                if (selectionMask[i]) {
                    subSet.push_back(superSet->at(i));
                }
            }
            assert(subSet.size() == subSetSize);

            yield(std::move(subSet));
        } while (std::prev_permutation(
            selectionMask.begin(), selectionMask.end()));
    });
}
