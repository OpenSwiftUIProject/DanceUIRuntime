// Copyright (c) 2025 ByteDance Ltd. and/or its affiliates
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef EnvironmentVariable_hpp
#define EnvironmentVariable_hpp

#include "DanceUI_Debug.hpp"
#include <stdlib.h>
#include <type_traits>
#include <functional>

namespace DanceUI {


namespace Environment {

template <typename Ty, Ty Default>
struct convert {
public:
    Ty operator()(const char *__v) const noexcept {
        return Default;
    }
};

template <typename ResTy, ResTy Default>
struct convert<std::enable_if_t<std::is_integral_v<ResTy>, ResTy>, Default> {
public:
    ResTy operator()(const char *v) const noexcept {
        if (!v) {
            return Default;
        }
        return static_cast<ResTy>(atoll(v));
    }
};

template <>
struct convert<const char *, nullptr> {
public:
    const char *operator()(const char *__v) const noexcept {
        return __v;
    }
};

template <const char *Default>
struct convert<const char *, Default> {
public:
    const char *operator()(const char *__v) const noexcept {
        return __v == nullptr ? Default : __v;
    }
};

template <bool Default>
struct convert<bool, Default> {
public:
    bool operator()(const char *v) const noexcept {
        if (!v) {
            return Default;
        }
        return atoi(v) != 0;
    }
};

};

template <typename ResTy, const char *Key, ResTy Default, typename C = Environment::convert<ResTy, Default>>
class EnvironmentVariable final {
public:

    using value_type = ResTy;

    static_assert(Key != nullptr);

    DANCE_UI_INLINE
    EnvironmentVariable() noexcept {
        const char *value = getenv(Key);
        C c;
        _value = c(value);
    }

    DANCE_UI_INLINE
    ~EnvironmentVariable() noexcept {
    }

    DANCE_UI_INLINE
    const value_type get_value() const noexcept {
        return _value;
    }

    DANCE_UI_INLINE
    operator value_type() const noexcept {
        return _value;
    }

    DANCE_UI_INLINE
    EnvironmentVariable &operator=(const value_type &value) noexcept {
        _value = value;
        return *this;
    }

private:
    value_type _value;
};

}

#endif
