// This file is part of xscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include <functional>
#include <optional>

// On-demand value computation and then storage, not thread-safe by now.
template<class Value>
class Lazy {
public:
    using Initializer = std::function<Value()>;

    Lazy(Initializer init)
        : init_(std::move(init))
    {}

    const Value& operator()() const
    {
        if (!value_) {
            value_ = init_();
        }
        return *value_;
    }

private:
    const Initializer init_;
    mutable std::optional<Value> value_;
};
