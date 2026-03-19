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

#import <Foundation/Foundation.h>
#include <string>
#include "../../DanceUIRuntime/Sources/DanceUIRuntime/include/vector.hpp"

static const std::string anchor = "";

using namespace DanceUI;

namespace {
class NotPODTester final {
public:

    NotPODTester(size_t &dealloc_counter,
                 size_t value = arc4random() % 1000000) noexcept :
    _dealloc_counter(dealloc_counter),
    _value(value) {
        _anchor = new size_t(value);
    }

    ~NotPODTester() noexcept {
        if (_anchor == nullptr) {
            return;
        }
        assert(*_anchor == _value);
        delete _anchor;
        _anchor = nullptr;
        _dealloc_counter += 1;
    }

    NotPODTester(const NotPODTester &instance) noexcept :
    _dealloc_counter(instance._dealloc_counter),
    _value(instance._value) {
        _anchor = new size_t(*instance._anchor);
    }

    NotPODTester(NotPODTester &&instance) noexcept :
    _dealloc_counter(instance._dealloc_counter) {
        _anchor = instance._anchor;
        _value = instance._value;
        instance._anchor = nullptr;
    }

    size_t getValue() const noexcept {
        return _anchor != nullptr ? *_anchor : ~0;
    }

private:
    size_t &_dealloc_counter;
    size_t *_anchor;
    size_t _value;
};

class PODTester final {
public:

    PODTester(size_t &dealloc_counter,
              size_t value = arc4random() % 1000000) noexcept :
    _dealloc_counter(dealloc_counter),
    _value(value) {
        _anchor = size_t(value);
    }

    ~PODTester() noexcept {
        if (_anchor == ~0) {
            return;
        }
        assert(_anchor == _value);
        _anchor = ~0;
        _dealloc_counter += 1;
    }

    PODTester(const PODTester &instance) noexcept :
    _dealloc_counter(instance._dealloc_counter),
    _value(instance._value) {
        _anchor = size_t(instance._anchor);
    }

    PODTester(PODTester &&instance) noexcept :
    _dealloc_counter(instance._dealloc_counter) {
        _anchor = instance._anchor;
        _value = instance._value;
        instance._anchor = ~0;
    }

    size_t getValue() const noexcept {
        return _anchor != ~0 ? _anchor : ~0;
    }

private:
    size_t &_dealloc_counter;
    size_t _anchor;
    size_t _value;
};
}

FOUNDATION_EXPORT
void test_pod_heapOnly(void) {
    size_t dealloc_counter = 0;
    const size_t loop_count = 0x200;
    const size_t repeat_count = 0x1000;

    for (size_t idx = 0; idx < repeat_count; idx ++) {
        vector<PODTester, 0> buffer;
        for (size_t idx = 0; idx < loop_count; idx++) {
            buffer.push_back(PODTester(dealloc_counter, idx));
            assert(!buffer.is_using_stack_buffer_now());
        }
    }

    assert(dealloc_counter == repeat_count * loop_count);
}

FOUNDATION_EXPORT
void test_pod_stackOnly(void) {
    size_t dealloc_counter = 0;
    const size_t loop_count = 0x200;
    const size_t repeat_count = 0x1000;

    for (size_t idx = 0; idx < repeat_count; idx ++) {
        vector<PODTester, loop_count> buffer;
        for (size_t idx = 0; idx < loop_count; idx++) {
            buffer.push_back(PODTester(dealloc_counter, idx));
            assert(buffer.is_using_stack_buffer_now());
        }
    }

    assert(dealloc_counter == repeat_count * loop_count);
}
