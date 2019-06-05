// This file is part of xdscribe
//
// Copyright (C) 2019 Sergey Karpukhin <contact@kserz.rocks>
//
// Licensed under GNU General Public License version 3.
// Full license text is available in LICENSE file.

#pragma once

#include <cassert>
#include <functional>
#include <type_traits>
#include <vector>

// A stream-like way of returning huge data from functions.
// Allows us to not store the results in memory where it is reasonable
// and consume them one by one right after the construction.
//
// NB. Generators returned from the objects almost always store
// pointers to them, take care of lifetime.
// TODO: Add size estimation?
template<class Value>
class Generator {
public:
    using ValueType = typename std::decay<Value>::type;
    using VectorStoragePointer = typename std::conditional<
        std::is_const<std::remove_reference_t<Value>>::value,
        const std::vector<ValueType>*,
        std::vector<ValueType>*>::type;

    using Callback = std::function<void(Value)>;
    using Source = std::function<void(const Callback&)>;

    Generator(Source source)
        : source_(std::move(source))
        , storage_(nullptr)
    {}
    // Faster iteration over vectors
    Generator(VectorStoragePointer storage)
        : storage_(storage)
    {}

    // TODO: allow break in the middle of the loop
    template<class Callback_>
    void process(const Callback_& callback) const
    {
        if (storage_) {
            for (auto&& value : *storage_) {
                callback(std::forward<Value>(value));
            }
        } else {
            source_(callback);
        }
    }

private:
    const Source source_;
    VectorStoragePointer const storage_;
};

template<class DstVal, class SrcVal, class Compose>
Generator<DstVal> compositeGenerator(Generator<SrcVal> srcGen, Compose compose)
{
    return Generator<DstVal>(
        [srcGen = std::move(srcGen), compose = std::move(compose)]
                (auto&& yield)
        {
            srcGen.process([&] (SrcVal&& value)
            {
                compose(std::forward<SrcVal>(value), yield);
            });
        });
}

template<class DstVal, class SrcVal, class Map>
Generator<DstVal> mapGenerator(Generator<SrcVal> srcGen, Map map)
{
    return compositeGenerator<DstVal, SrcVal>(
        std::move(srcGen),
        [map = std::move(map)] (SrcVal&& value, auto&& yield) {
            yield(map(std::forward<SrcVal>(value)));
        });
}

template<class Value, class PassFilter>
Generator<Value> filterGenerator(Generator<Value> srcGen, PassFilter passFilter)
{
    return compositeGenerator<Value, Value>(
        std::move(srcGen),
        [passFilter = std::move(passFilter)] (Value&& value, auto&& yield) {
            if (passFilter(value)) {
                yield(std::forward<Value>(value));
            }
        });
}
