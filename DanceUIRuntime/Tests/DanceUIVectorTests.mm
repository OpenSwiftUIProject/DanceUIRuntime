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

#import <XCTest/XCTest.h>

#include "../../DanceUIRuntime/Sources/DanceUIRuntime/include/DanceUIRuntime/vector.hpp"
#include <vector>

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
        _value = arc4random() % 1000000 + 1000000;
        _dealloc_counter += 1;
    }

    NotPODTester(const NotPODTester &instance) noexcept :
    _dealloc_counter(instance._dealloc_counter),
    _anchor(nullptr),
    _value(0) {
        *this = instance;
    }

    NotPODTester(NotPODTester &&instance) noexcept :
    _dealloc_counter(instance._dealloc_counter) {
        _anchor = instance._anchor;
        _value = instance._value;
        instance._anchor = nullptr;
    }

    NotPODTester &operator=(const NotPODTester &rhs) noexcept {
        if (this != &rhs) {
            _dealloc_counter = rhs._dealloc_counter;
            _value = rhs._value;
            if (_anchor) {
                delete _anchor;
                _anchor = nullptr;
            }
            if (rhs._anchor  != nullptr) {
                _anchor = new size_t(*rhs._anchor);
            }
        }
        return *this;
    }

    NotPODTester &operator=(NotPODTester &&rhs) noexcept {
        if (this != &rhs) {
            _dealloc_counter = rhs._dealloc_counter;
            _value = rhs._value;
            if (_anchor) {
                delete _anchor;
            }
            _anchor = rhs._anchor;
            rhs._anchor = nullptr;
        }
        return *this;
    }

    bool operator==(const NotPODTester &rhs) const noexcept {
        if (this == &rhs) {
            return true;
        }
        return getValue() == rhs.getValue();
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
        _value = arc4random() % 1000000 + 1000000;
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

@interface DanceUIVectorTests : XCTestCase

@end

@implementation DanceUIVectorTests

- (void)testStackBasedVector_distance_of_iterators {
    size_t dealloc_counter = 0;
    const size_t stack_buffer_capacity = 0x10;
    using size_type = vector<NotPODTester, stack_buffer_capacity>::size_type;

    const size_type loop_count = stack_buffer_capacity;

    {
        vector<NotPODTester, stack_buffer_capacity> buffer;
        XCTAssertTrue(buffer.is_stack_buffer_available);
        XCTAssertTrue(buffer.default_alloc_size == stack_buffer_capacity);

        for (size_type idx = 0; idx < loop_count; idx ++) {
            buffer.push_back(NotPODTester(dealloc_counter, idx));
        }
        for (size_type idx = 0; idx < loop_count; idx ++) {
            XCTAssertTrue(buffer[idx].getValue() == idx);
        }

        XCTAssertTrue(buffer.is_using_stack_buffer_now());
        auto distance = std::distance(buffer.begin(), buffer.end());

        XCTAssertEqual(distance,
                       buffer.size());
    }
    XCTAssertTrue(dealloc_counter == loop_count);
}

- (void)testStackBasedVector_stack_only {

    size_t dealloc_counter = 0;
    const size_t stack_buffer_capacity = 0x10;
    using size_type = vector<NotPODTester, stack_buffer_capacity>::size_type;

    const size_type loop_count = stack_buffer_capacity;

    {
        vector<NotPODTester, stack_buffer_capacity> buffer;
        XCTAssertTrue(buffer.is_stack_buffer_available);
        XCTAssertTrue(buffer.default_alloc_size == stack_buffer_capacity);

        for (size_type idx = 0; idx < loop_count; idx ++) {
            buffer.push_back(NotPODTester(dealloc_counter, idx));
        }
        for (size_type idx = 0; idx < loop_count; idx ++) {
            XCTAssertTrue(buffer[idx].getValue() == idx);
        }

        XCTAssertTrue(buffer.is_using_stack_buffer_now());
    }
    XCTAssertTrue(dealloc_counter == loop_count);
}

- (void)testStackBasedVector_stack_only_assign_to_stack_only {

    size_t dealloc_counter = 0;
    const size_t stack_buffer_capacity = 0x10;
    using size_type = vector<NotPODTester, stack_buffer_capacity>::size_type;

    const size_type loop_count = stack_buffer_capacity;

    {
        vector<NotPODTester, stack_buffer_capacity> buffer;
        vector<NotPODTester, stack_buffer_capacity> buffer2;
        XCTAssertTrue(buffer.is_stack_buffer_available);
        XCTAssertTrue(buffer.default_alloc_size == stack_buffer_capacity);

        for (size_type idx = 0; idx < loop_count; idx ++) {
            buffer.push_back(NotPODTester(dealloc_counter, idx));
        }
        for (size_type idx = 0; idx < loop_count; idx ++) {
            XCTAssertTrue(buffer[idx].getValue() == idx);
        }
        XCTAssertTrue(buffer.is_using_stack_buffer_now());

        buffer2 = buffer;
        XCTAssertTrue(buffer2.is_using_stack_buffer_now());
        XCTAssertTrue(buffer2.size() == buffer.size());
        XCTAssertTrue(buffer2 == buffer);
    }
    XCTAssertEqual(dealloc_counter, loop_count * 2);
}

- (void)testStackBasedVector_stack_only_move_to_stack_only {

    size_t dealloc_counter = 0;
    const size_t stack_buffer_capacity = 0x10;
    using size_type = vector<NotPODTester, stack_buffer_capacity>::size_type;

    const size_type loop_count = stack_buffer_capacity;

    {
        vector<NotPODTester, stack_buffer_capacity> buffer;
        vector<NotPODTester, stack_buffer_capacity> buffer2;
        XCTAssertTrue(buffer.is_stack_buffer_available);
        XCTAssertTrue(buffer.default_alloc_size == stack_buffer_capacity);

        for (size_type idx = 0; idx < loop_count; idx ++) {
            buffer.push_back(NotPODTester(dealloc_counter, idx));
        }
        for (size_type idx = 0; idx < loop_count; idx ++) {
            XCTAssertTrue(buffer[idx].getValue() == idx);
        }
        XCTAssertTrue(buffer.is_using_stack_buffer_now());

        buffer2 = std::move(buffer);
        XCTAssertTrue(buffer2.is_using_stack_buffer_now());
        XCTAssertTrue(buffer2.size() == loop_count);
        XCTAssertTrue(buffer.empty());
    }
    XCTAssertEqual(dealloc_counter, loop_count);
}

- (void)testStackBasedVector_stack_to_heap {
    size_t dealloc_counter = 0;
    const size_t stack_buffer_capacity = 0x10;
    using size_type = vector<NotPODTester, stack_buffer_capacity>::size_type;

    const size_type loop_count = 0x20;
    {
        vector<NotPODTester, stack_buffer_capacity> buffer;
        XCTAssertTrue(buffer.is_stack_buffer_available);
        XCTAssertTrue(buffer.default_alloc_size == stack_buffer_capacity);

        static_assert(loop_count > stack_buffer_capacity,
                      "loop count should be greater than stack_buffer_capacity");

        for (size_type idx = 0; idx < loop_count; idx ++) {
            buffer.push_back(NotPODTester(dealloc_counter, idx));
            if (idx < stack_buffer_capacity) {
                XCTAssertTrue(buffer.is_using_stack_buffer_now());
            } else {
                XCTAssertFalse(buffer.is_using_stack_buffer_now());
            }
        }
        for (size_type idx = 0; idx < loop_count; idx ++) {
            XCTAssertTrue(buffer[idx].getValue() == idx);
        }
        buffer.clear();
        XCTAssertTrue(buffer.is_using_stack_buffer_now());
    }
    XCTAssertTrue(dealloc_counter == loop_count);
}

- (void)testStackBasedVector_stack_to_heap_assign_to_stack {
    size_t dealloc_counter = 0;
    const size_t stack_buffer_capacity = 0x10;
    using size_type = vector<NotPODTester, stack_buffer_capacity>::size_type;

    const size_type loop_count = 0x20;
    {
        vector<NotPODTester, stack_buffer_capacity> buffer;
        vector<NotPODTester, stack_buffer_capacity> buffer2;
        XCTAssertTrue(buffer.is_stack_buffer_available);
        XCTAssertTrue(buffer.default_alloc_size == stack_buffer_capacity);

        static_assert(loop_count > stack_buffer_capacity,
                      "loop count should be greater than stack_buffer_capacity");

        for (size_type idx = 0; idx < loop_count; idx ++) {
            buffer.push_back(NotPODTester(dealloc_counter, idx));
            if (idx < stack_buffer_capacity) {
                XCTAssertTrue(buffer.is_using_stack_buffer_now());
            } else {
                XCTAssertFalse(buffer.is_using_stack_buffer_now());
            }
        }
        for (size_type idx = 0; idx < loop_count; idx ++) {
            XCTAssertTrue(buffer[idx].getValue() == idx);
        }
        buffer2 = buffer;
        XCTAssertFalse(buffer2.is_using_stack_buffer_now());
        XCTAssertTrue(buffer2.size() == buffer.size());
        XCTAssertTrue(buffer2 == buffer);

        buffer.clear();
        XCTAssertTrue(buffer.is_using_stack_buffer_now());
    }
    XCTAssertEqual(dealloc_counter, loop_count * 2);
}

- (void)testStackBasedVector_stack_to_heap_move_to_stack {
    size_t dealloc_counter = 0;
    const size_t stack_buffer_capacity = 0x10;
    using size_type = vector<NotPODTester, stack_buffer_capacity>::size_type;

    const size_type loop_count = 0x20;
    {
        vector<NotPODTester, stack_buffer_capacity> buffer;
        vector<NotPODTester, stack_buffer_capacity> buffer2;
        XCTAssertTrue(buffer.is_stack_buffer_available);
        XCTAssertTrue(buffer.default_alloc_size == stack_buffer_capacity);

        static_assert(loop_count > stack_buffer_capacity,
                      "loop count should be greater than stack_buffer_capacity");

        for (size_type idx = 0; idx < loop_count; idx ++) {
            buffer.push_back(NotPODTester(dealloc_counter, idx));
            if (idx < stack_buffer_capacity) {
                XCTAssertTrue(buffer.is_using_stack_buffer_now());
            } else {
                XCTAssertFalse(buffer.is_using_stack_buffer_now());
            }
        }
        for (size_type idx = 0; idx < loop_count; idx ++) {
            XCTAssertTrue(buffer[idx].getValue() == idx);
        }
        buffer2 = std::move(buffer);
        XCTAssertFalse(buffer2.is_using_stack_buffer_now());
        XCTAssertTrue(buffer2.size() == loop_count);

        for (size_type idx = 0; idx < loop_count; idx ++) {
            XCTAssertTrue(buffer2[idx].getValue() == idx);
        }

        XCTAssertTrue(buffer.is_using_stack_buffer_now());
    }
    XCTAssertEqual(dealloc_counter, loop_count);
}

- (void)testStackBasedVector_stack_based_on_stack_copy_assign_to_stack_based_on_stack {
    size_t dealloc_counter = 0;
    const size_t stack_buffer_capacity = 0x10;
    using size_type = vector<NotPODTester, stack_buffer_capacity>::size_type;

    const size_type loop_count = stack_buffer_capacity;

    {
        vector<NotPODTester, stack_buffer_capacity> rhs;
        XCTAssertTrue(rhs.is_stack_buffer_available);
        XCTAssertTrue(rhs.default_alloc_size == stack_buffer_capacity);

        for (size_type idx = 0; idx < loop_count; idx ++) {
            rhs.push_back(NotPODTester(dealloc_counter, idx));
            XCTAssertTrue(rhs.is_using_stack_buffer_now());
        }

        vector<NotPODTester, stack_buffer_capacity> lhs;
        XCTAssertTrue(lhs.is_stack_buffer_available);
        XCTAssertTrue(lhs.default_alloc_size == stack_buffer_capacity);
        XCTAssertTrue(lhs.is_using_stack_buffer_now());

        lhs = rhs;
        XCTAssertTrue(lhs.is_using_stack_buffer_now());
        XCTAssertTrue(rhs.is_using_stack_buffer_now());

        for (size_type idx = 0; idx < loop_count; idx ++) {
            XCTAssertTrue(lhs[idx].getValue() == idx);
            XCTAssertTrue(lhs.is_using_stack_buffer_now());
        }
    }
    XCTAssertTrue(dealloc_counter == 2 * loop_count);
}

- (void)testStackBasedVector_stack_based_on_stack_move_assign_to_stack_based_on_stack {
    size_t dealloc_counter = 0;
    const size_t stack_buffer_capacity = 0x10;
    using size_type = vector<NotPODTester, stack_buffer_capacity>::size_type;

    const size_type loop_count = stack_buffer_capacity;

    {
        vector<NotPODTester, stack_buffer_capacity> rhs;
        XCTAssertTrue(rhs.is_stack_buffer_available);
        XCTAssertTrue(rhs.default_alloc_size == stack_buffer_capacity);

        for (size_type idx = 0; idx < loop_count; idx ++) {
            rhs.push_back(NotPODTester(dealloc_counter, idx));
            XCTAssertTrue(rhs.is_using_stack_buffer_now());
        }

        vector<NotPODTester, stack_buffer_capacity> lhs;
        XCTAssertTrue(lhs.is_stack_buffer_available);
        XCTAssertTrue(lhs.default_alloc_size == stack_buffer_capacity);
        XCTAssertTrue(lhs.is_using_stack_buffer_now());

        lhs = std::move(rhs);
        XCTAssertTrue(lhs.is_using_stack_buffer_now());
        XCTAssertTrue(rhs.is_using_stack_buffer_now());

        for (size_type idx = 0; idx < loop_count; idx ++) {
            XCTAssertTrue(lhs[idx].getValue() == idx);
            XCTAssertTrue(lhs.is_using_stack_buffer_now());
        }
    }
    XCTAssertTrue(dealloc_counter == loop_count);
}

- (void)testStackBasedVector_stack_based_on_stack_copy_assign_to_stack_based_but_on_heap {
    size_t dealloc_counter = 0;
    const size_t stack_buffer_capacity = 0x10;
    using size_type = vector<NotPODTester, stack_buffer_capacity>::size_type;

    const size_type loop_count = 0x20;
    {
        vector<NotPODTester, loop_count> rhs;
        XCTAssertTrue(rhs.is_stack_buffer_available);
        XCTAssertTrue(rhs.default_alloc_size == loop_count);

        for (size_type idx = 0; idx < loop_count; idx ++) {
            rhs.push_back(NotPODTester(dealloc_counter, idx));
            XCTAssertTrue(rhs.is_using_stack_buffer_now());
        }

        vector<NotPODTester, stack_buffer_capacity> lhs;
        XCTAssertTrue(lhs.is_stack_buffer_available);
        XCTAssertTrue(lhs.default_alloc_size == stack_buffer_capacity);
        XCTAssertTrue(lhs.is_using_stack_buffer_now());

        for (size_type idx = 0; idx < loop_count; idx ++) {
            lhs.push_back(NotPODTester(dealloc_counter, idx));
            if (idx < stack_buffer_capacity) {
                XCTAssertTrue(lhs.is_using_stack_buffer_now());
            } else {
                XCTAssertFalse(lhs.is_using_stack_buffer_now());
            }
        }

        lhs = rhs;
        XCTAssertFalse(lhs.is_using_stack_buffer_now());
        XCTAssertTrue(rhs.is_using_stack_buffer_now());

        for (size_type idx = 0; idx < loop_count; idx ++) {
            XCTAssertTrue(lhs[idx].getValue() == idx);
            XCTAssertFalse(lhs.is_using_stack_buffer_now());
        }
    }
    XCTAssertTrue(dealloc_counter == loop_count * 3);
}

- (void)testStackBasedVector_stack_based_on_stack_move_assign_to_stack_based_but_on_heap {
    size_t dealloc_counter = 0;
    const size_t stack_buffer_capacity = 0x10;
    using size_type = vector<NotPODTester, stack_buffer_capacity>::size_type;

    const size_type loop_count = 0x20;
    {
        vector<NotPODTester, loop_count> rhs;
        XCTAssertTrue(rhs.is_stack_buffer_available);
        XCTAssertTrue(rhs.default_alloc_size == loop_count);

        for (size_type idx = 0; idx < loop_count; idx ++) {
            rhs.push_back(NotPODTester(dealloc_counter, idx));
            XCTAssertTrue(rhs.is_using_stack_buffer_now());
        }

        vector<NotPODTester, stack_buffer_capacity> lhs;
        XCTAssertTrue(lhs.is_stack_buffer_available);
        XCTAssertTrue(lhs.default_alloc_size == stack_buffer_capacity);
        XCTAssertTrue(lhs.is_using_stack_buffer_now());

        for (size_type idx = 0; idx < loop_count; idx ++) {
            lhs.push_back(NotPODTester(dealloc_counter, idx));
            if (idx < stack_buffer_capacity) {
                XCTAssertTrue(lhs.is_using_stack_buffer_now());
            } else {
                XCTAssertFalse(lhs.is_using_stack_buffer_now());
            }
        }

        lhs = std::move(rhs);
        XCTAssertFalse(lhs.is_using_stack_buffer_now());
        XCTAssertTrue(rhs.is_using_stack_buffer_now());

        for (size_type idx = 0; idx < loop_count; idx ++) {
            XCTAssertTrue(lhs[idx].getValue() == idx);
            XCTAssertFalse(lhs.is_using_stack_buffer_now());
        }
    }
    XCTAssertTrue(dealloc_counter == loop_count * 2);
}

- (void)testStackBasedVector_stack_based_on_stack_copy_assign_to_heap_only {
    size_t dealloc_counter = 0;
    const size_t stack_buffer_capacity = 0x10;
    using size_type = vector<NotPODTester, stack_buffer_capacity>::size_type;

    const size_type loop_count = stack_buffer_capacity;
    {
        vector<NotPODTester, stack_buffer_capacity> rhs;
        XCTAssertTrue(rhs.is_stack_buffer_available);
        XCTAssertTrue(rhs.default_alloc_size == stack_buffer_capacity);

        for (size_type idx = 0; idx < loop_count; idx ++) {
            rhs.push_back(NotPODTester(dealloc_counter, idx));
            XCTAssertTrue(rhs.is_using_stack_buffer_now());
        }

        vector<NotPODTester, 0> lhs;
        XCTAssertFalse(lhs.is_stack_buffer_available);
        XCTAssertTrue(lhs.default_alloc_size == 4);
        XCTAssertFalse(lhs.is_using_stack_buffer_now());

        for (size_type idx = 0; idx < loop_count; idx ++) {
            lhs.push_back(NotPODTester(dealloc_counter, idx));
            XCTAssertFalse(lhs.is_using_stack_buffer_now());
        }

        lhs = rhs;
        XCTAssertFalse(lhs.is_using_stack_buffer_now());
        XCTAssertTrue(rhs.is_using_stack_buffer_now());

        for (size_type idx = 0; idx < loop_count; idx ++) {
            XCTAssertTrue(lhs[idx].getValue() == idx);
            XCTAssertFalse(lhs.is_using_stack_buffer_now());
        }
    }
    XCTAssertTrue(dealloc_counter == loop_count * 3);
}

- (void)testStackBasedVector_stack_based_on_stack_move_assign_to_heap_only {
    size_t dealloc_counter = 0;
    const size_t stack_buffer_capacity = 0x10;
    using size_type = vector<NotPODTester, stack_buffer_capacity>::size_type;

    const size_type loop_count = stack_buffer_capacity;
    {
        vector<NotPODTester, stack_buffer_capacity> rhs;
        XCTAssertTrue(rhs.is_stack_buffer_available);
        XCTAssertTrue(rhs.default_alloc_size == stack_buffer_capacity);

        for (size_type idx = 0; idx < loop_count; idx ++) {
            rhs.push_back(NotPODTester(dealloc_counter, idx));
            XCTAssertTrue(rhs.is_using_stack_buffer_now());
        }

        vector<NotPODTester, 0> lhs;
        XCTAssertFalse(lhs.is_stack_buffer_available);
        XCTAssertTrue(lhs.default_alloc_size == 4);
        XCTAssertFalse(lhs.is_using_stack_buffer_now());

        for (size_type idx = 0; idx < loop_count; idx ++) {
            lhs.push_back(NotPODTester(dealloc_counter, idx));
            XCTAssertFalse(lhs.is_using_stack_buffer_now());
        }

        lhs = std::move(rhs);
        XCTAssertFalse(lhs.is_using_stack_buffer_now());
        XCTAssertTrue(rhs.is_using_stack_buffer_now());

        for (size_type idx = 0; idx < loop_count; idx ++) {
            XCTAssertTrue(lhs[idx].getValue() == idx);
            XCTAssertFalse(lhs.is_using_stack_buffer_now());
        }
    }
    XCTAssertTrue(dealloc_counter == loop_count * 2);
}

- (void)testStackBasedVector_stack_based_on_stack_erase_from_begin {
    size_t dealloc_counter = 0;
    const size_t stack_buffer_capacity = 0x10;
    using size_type = vector<NotPODTester, stack_buffer_capacity>::size_type;

    const size_type loop_count = 0x20;
    {
        vector<NotPODTester, stack_buffer_capacity> buffer;
        XCTAssertTrue(buffer.is_stack_buffer_available);
        XCTAssertTrue(buffer.default_alloc_size == stack_buffer_capacity);

        static_assert(loop_count > stack_buffer_capacity,
                      "loop count should be greater than stack_buffer_capacity");

        for (size_type idx = 0; idx < loop_count; idx ++) {
            buffer.push_back(NotPODTester(dealloc_counter, idx));
            if (idx < stack_buffer_capacity) {
                XCTAssertTrue(buffer.is_using_stack_buffer_now());
            } else {
                XCTAssertFalse(buffer.is_using_stack_buffer_now());
            }
        }
        for (size_type idx = 0; idx < loop_count; idx ++) {
            XCTAssertTrue(buffer[idx].getValue() == idx);
        }
        buffer.erase(buffer.begin(), buffer.end());
        XCTAssertTrue(buffer.empty());
        XCTAssertTrue(buffer.size() == 0);
        XCTAssertFalse(buffer.is_using_stack_buffer_now());
    }
    XCTAssertTrue(dealloc_counter == loop_count);
}

- (void)testStackBasedVector_stack_based_on_stack_erase_with_firstplace_using_std_remove {
    size_t dealloc_counter = 0;
    const size_t stack_buffer_capacity = 0x10;
    using size_type = vector<NotPODTester, stack_buffer_capacity>::size_type;

    const size_type loop_count = 0x20;
    {
        vector<NotPODTester, stack_buffer_capacity> buffer;
        XCTAssertTrue(buffer.is_stack_buffer_available);
        XCTAssertTrue(buffer.default_alloc_size == stack_buffer_capacity);

        static_assert(loop_count > stack_buffer_capacity,
                      "loop count should be greater than stack_buffer_capacity");

        const size_type remove_idx = 0;

        for (size_type idx = 0; idx < loop_count; idx ++) {
            buffer.push_back(NotPODTester(dealloc_counter, idx));
            if (idx < stack_buffer_capacity) {
                XCTAssertTrue(buffer.is_using_stack_buffer_now());
            } else {
                XCTAssertFalse(buffer.is_using_stack_buffer_now());
            }
        }
        for (size_type idx = 0; idx < loop_count; idx ++) {
            XCTAssertTrue(buffer[idx].getValue() == idx);
        }

        const NotPODTester tester(dealloc_counter, remove_idx);
        {
            auto removed = std::remove(buffer.begin(), buffer.end(), std::move(tester));
            if (removed != buffer.end()) {
                buffer.erase(removed);
            }
        }

        XCTAssertFalse(buffer.empty());
        XCTAssertTrue(buffer.size() == loop_count - 1);
        XCTAssertFalse(buffer.is_using_stack_buffer_now());

        for (size_type idx = 0; idx < loop_count - 1; idx ++) {
            XCTAssertTrue(buffer[idx].getValue() != remove_idx);
        }
    }
    XCTAssertEqual(dealloc_counter, loop_count);
}

- (void)testStackBasedVector_stack_based_on_stack_erase_with_lastplace_using_std_remove {
    size_t dealloc_counter = 0;
    const size_t stack_buffer_capacity = 0x10;
    using size_type = vector<NotPODTester, stack_buffer_capacity>::size_type;

    const size_type loop_count = 0x20;
    {
        vector<NotPODTester, stack_buffer_capacity> buffer;
        XCTAssertTrue(buffer.is_stack_buffer_available);
        XCTAssertTrue(buffer.default_alloc_size == stack_buffer_capacity);

        static_assert(loop_count > stack_buffer_capacity,
                      "loop count should be greater than stack_buffer_capacity");

        const size_type remove_idx = loop_count - 1;

        for (size_type idx = 0; idx < loop_count; idx ++) {
            buffer.push_back(NotPODTester(dealloc_counter, idx));
            if (idx < stack_buffer_capacity) {
                XCTAssertTrue(buffer.is_using_stack_buffer_now());
            } else {
                XCTAssertFalse(buffer.is_using_stack_buffer_now());
            }
        }
        for (size_type idx = 0; idx < loop_count; idx ++) {
            XCTAssertTrue(buffer[idx].getValue() == idx);
        }

        const NotPODTester tester(dealloc_counter, remove_idx);
        {
            auto removed = std::remove(buffer.begin(), buffer.end(), std::move(tester));
            if (removed != buffer.end()) {
                buffer.erase(removed);
            }
        }

        XCTAssertFalse(buffer.empty());
        XCTAssertTrue(buffer.size() == loop_count - 1);
        XCTAssertFalse(buffer.is_using_stack_buffer_now());

        for (size_type idx = 0; idx < loop_count - 1; idx ++) {
            XCTAssertTrue(buffer[idx].getValue() != remove_idx);
        }
    }
    XCTAssertEqual(dealloc_counter, loop_count + 1);
}

- (void)testStackBasedVector_stack_based_on_stack_erase_with_random_using_std_remove {
    size_t dealloc_counter = 0;
    const size_t stack_buffer_capacity = 0x10;
    using size_type = vector<NotPODTester, stack_buffer_capacity>::size_type;

    const size_type loop_count = 0x20;
    {
        vector<NotPODTester, stack_buffer_capacity> buffer;
        XCTAssertTrue(buffer.is_stack_buffer_available);
        XCTAssertEqual(buffer.default_alloc_size, stack_buffer_capacity);

        static_assert(loop_count > stack_buffer_capacity,
                      "loop count should be greater than stack_buffer_capacity");

        const size_type remove_idx = arc4random() % loop_count;

        for (size_type idx = 0; idx < loop_count; idx ++) {
            buffer.push_back(NotPODTester(dealloc_counter, idx));
            if (idx < stack_buffer_capacity) {
                XCTAssertTrue(buffer.is_using_stack_buffer_now());
            } else {
                XCTAssertFalse(buffer.is_using_stack_buffer_now());
            }
        }
        for (size_type idx = 0; idx < loop_count; idx ++) {
            XCTAssertEqual(buffer[idx].getValue(), idx);
        }

        const NotPODTester tester(dealloc_counter, remove_idx);
        {
            auto removed = std::remove(buffer.begin(), buffer.end(), std::move(tester));
            if (removed != buffer.end()) {
                buffer.erase(removed);
            }
        }

        XCTAssertFalse(buffer.empty());
        XCTAssertEqual(buffer.size(), loop_count - 1);
        XCTAssertFalse(buffer.is_using_stack_buffer_now());

        for (size_type idx = 0; idx < loop_count - 1; idx ++) {
            if (idx < remove_idx) {
                XCTAssertEqual(buffer[idx].getValue(), idx);
            } else {
                XCTAssertEqual(buffer[idx].getValue(), idx + 1);
            }
        }
    }
    XCTAssertEqual(dealloc_counter, loop_count);
}

- (void)testStackBasedVector_stack_based_on_stack_erase_with_not_found_using_std_remove {
    size_t dealloc_counter = 0;
    const size_t stack_buffer_capacity = 0x10;
    using size_type = vector<NotPODTester, stack_buffer_capacity>::size_type;

    const size_type loop_count = 0x20;
    {
        vector<NotPODTester, stack_buffer_capacity> buffer;
        XCTAssertTrue(buffer.is_stack_buffer_available);
        XCTAssertTrue(buffer.default_alloc_size == stack_buffer_capacity);

        static_assert(loop_count > stack_buffer_capacity,
                      "loop count should be greater than stack_buffer_capacity");
        const size_type remove_idx = arc4random() + loop_count;

        for (size_type idx = 0; idx < loop_count; idx ++) {
            buffer.push_back(NotPODTester(dealloc_counter, idx));
            if (idx < stack_buffer_capacity) {
                XCTAssertTrue(buffer.is_using_stack_buffer_now());
            } else {
                XCTAssertFalse(buffer.is_using_stack_buffer_now());
            }
        }
        for (size_type idx = 0; idx < loop_count; idx ++) {
            XCTAssertTrue(buffer[idx].getValue() == idx);
        }

        const NotPODTester tester(dealloc_counter, remove_idx);

        {
            auto removed = std::remove(buffer.begin(), buffer.end(), std::move(tester));
            if (removed != buffer.end()) {
                buffer.erase(removed);
            }
        }

        XCTAssertFalse(buffer.empty());
        XCTAssertTrue(buffer.size() == loop_count);
        XCTAssertFalse(buffer.is_using_stack_buffer_now());

        for (size_type idx = 0; idx < loop_count; idx ++) {
            XCTAssertTrue(buffer[idx].getValue() == idx);
        }
    }
    XCTAssertEqual(dealloc_counter, loop_count + 1);
}

- (void)testStackBasedVector_stack_based_on_stack_insert_element_from_begin_position {
    size_t dealloc_counter = 0;
    const size_t stack_buffer_capacity = 0x10;
    using size_type = vector<NotPODTester, stack_buffer_capacity>::size_type;

    const size_type loop_count = stack_buffer_capacity;
    {
        vector<NotPODTester, stack_buffer_capacity> buffer;
        XCTAssertTrue(buffer.is_stack_buffer_available);
        XCTAssertTrue(buffer.default_alloc_size == stack_buffer_capacity);

        static_assert(loop_count == stack_buffer_capacity,
                      "loop count should be equals to stack_buffer_capacity");

        for (size_type idx = 0; idx < loop_count; idx ++) {
            XCTAssertTrue(buffer.size() == idx);
            buffer.insert(buffer.begin(), {dealloc_counter, idx});
            XCTAssertEqual(dealloc_counter, 0);
        }
        XCTAssertTrue(buffer.size() == loop_count);

        for (size_type idx = 0; idx < loop_count; idx ++) {
            const auto lhs = buffer[idx].getValue();
            const auto rhs = loop_count - 1 - idx;
            XCTAssertEqual(lhs, rhs);
        }
        buffer.erase(buffer.begin(), buffer.end());
        XCTAssertTrue(buffer.empty());
        XCTAssertTrue(buffer.size() == 0);
        XCTAssertTrue(buffer.is_using_stack_buffer_now());
    }
    XCTAssertEqual(dealloc_counter, loop_count);
}

- (void)testStackBasedVector_stack_based_on_stack_insert_element_from_end_position {
    size_t dealloc_counter = 0;
    const size_t stack_buffer_capacity = 0x10;
    using size_type = vector<NotPODTester, stack_buffer_capacity>::size_type;

    const size_type loop_count = stack_buffer_capacity;
    {
        vector<NotPODTester, stack_buffer_capacity> buffer;
        XCTAssertTrue(buffer.is_stack_buffer_available);
        XCTAssertTrue(buffer.default_alloc_size == stack_buffer_capacity);

        static_assert(loop_count == stack_buffer_capacity,
                      "loop count should be equals to stack_buffer_capacity");

        for (size_type idx = 0; idx < loop_count; idx ++) {
            XCTAssertTrue(buffer.size() == idx);
            buffer.insert(buffer.end(), {dealloc_counter, idx});
            XCTAssertEqual(dealloc_counter, 0);
        }
        XCTAssertTrue(buffer.size() == loop_count);

        for (size_type idx = 0; idx < loop_count; idx ++) {
            const auto lhs = buffer[idx].getValue();
            const auto rhs = idx;
            XCTAssertEqual(lhs, rhs);
        }
        buffer.erase(buffer.begin(), buffer.end());
        XCTAssertTrue(buffer.empty());
        XCTAssertTrue(buffer.size() == 0);
        XCTAssertTrue(buffer.is_using_stack_buffer_now());
    }
    XCTAssertEqual(dealloc_counter, loop_count);
}

- (void)testStackBasedVector_stack_based_on_stack_insert_element_from_random_position {
    size_t dealloc_counter = 0;
    const size_t stack_buffer_capacity = 0x10;
    using size_type = vector<NotPODTester, stack_buffer_capacity>::size_type;

    const size_type loop_count = stack_buffer_capacity;
    const size_type element_idx = arc4random() % stack_buffer_capacity;
    {
        vector<NotPODTester, stack_buffer_capacity> buffer;
        XCTAssertTrue(buffer.is_stack_buffer_available);
        XCTAssertTrue(buffer.default_alloc_size == stack_buffer_capacity);

        static_assert(loop_count == stack_buffer_capacity,
                      "loop count should be equals to stack_buffer_capacity");

        for (size_type idx = 0; idx < element_idx; idx ++) {
            XCTAssertTrue(buffer.size() == idx);
            buffer.insert(buffer.begin() + idx, {dealloc_counter, idx});
            XCTAssertEqual(dealloc_counter, 0);
        }

        for (size_type idx = 0; idx < loop_count - element_idx; idx ++) {
            XCTAssertTrue(buffer.size() == element_idx + idx);
            buffer.insert(buffer.begin() + element_idx, {
                dealloc_counter, idx + element_idx
            });
            XCTAssertEqual(dealloc_counter, 0);
        }
        XCTAssertTrue(buffer.size() == loop_count);

        for (size_type idx = 0; idx < element_idx; idx ++) {
            const auto lhs = buffer[idx].getValue();
            const auto rhs = idx;
            XCTAssertEqual(lhs, rhs);
        }

        for (size_type idx = element_idx; idx < loop_count; idx ++) {
            const auto lhs = buffer[idx].getValue();
            const auto rhs = loop_count - (idx - element_idx) - 1;
            XCTAssertEqual(lhs, rhs);
        }
        buffer.erase(buffer.begin(), buffer.end());
        XCTAssertTrue(buffer.empty());
        XCTAssertTrue(buffer.size() == 0);
        XCTAssertTrue(buffer.is_using_stack_buffer_now());
    }
    XCTAssertEqual(dealloc_counter, loop_count);
}

- (void)testStackBasedVector_stack_based_on_heap_insert_element_from_begin_position {
    size_t dealloc_counter = 0;
    const size_t stack_buffer_capacity = 0x10;
    using size_type = vector<NotPODTester, stack_buffer_capacity>::size_type;

    const size_type loop_count = 0x20;
    {
        vector<NotPODTester, stack_buffer_capacity> buffer;
        XCTAssertTrue(buffer.is_stack_buffer_available);
        XCTAssertTrue(buffer.default_alloc_size == stack_buffer_capacity);

        static_assert(loop_count > stack_buffer_capacity,
                      "loop count should be greater than stack_buffer_capacity");

        for (size_type idx = 0; idx < loop_count; idx ++) {
            XCTAssertTrue(buffer.size() == idx);
            buffer.insert(buffer.begin(), {dealloc_counter, idx});
            if (idx < stack_buffer_capacity) {
                XCTAssertTrue(buffer.is_using_stack_buffer_now());
            } else {
                XCTAssertFalse(buffer.is_using_stack_buffer_now());
            }
            XCTAssertEqual(dealloc_counter, 0);
        }
        XCTAssertTrue(buffer.size() == loop_count);

        for (size_type idx = 0; idx < loop_count; idx ++) {
            const auto lhs = buffer[idx].getValue();
            const auto rhs = loop_count - 1 - idx;
            XCTAssertEqual(lhs, rhs);
        }
        buffer.erase(buffer.begin(), buffer.end());
        XCTAssertTrue(buffer.empty());
        XCTAssertTrue(buffer.size() == 0);
        XCTAssertFalse(buffer.is_using_stack_buffer_now());
    }
    XCTAssertEqual(dealloc_counter, loop_count);
}

- (void)testStackBasedVector_stack_based_on_heap_insert_element_from_end_position {
    size_t dealloc_counter = 0;
    const size_t stack_buffer_capacity = 0x10;
    using size_type = vector<NotPODTester, stack_buffer_capacity>::size_type;

    const size_type loop_count = 0x20;
    {
        vector<NotPODTester, stack_buffer_capacity> buffer;
        XCTAssertTrue(buffer.is_stack_buffer_available);
        XCTAssertTrue(buffer.default_alloc_size == stack_buffer_capacity);

        static_assert(loop_count > stack_buffer_capacity,
                      "loop count should be greater than stack_buffer_capacity");

        for (size_type idx = 0; idx < loop_count; idx ++) {
            XCTAssertTrue(buffer.size() == idx);
            buffer.insert(buffer.end(), {dealloc_counter, idx});
            if (idx < stack_buffer_capacity) {
                XCTAssertTrue(buffer.is_using_stack_buffer_now());
            } else {
                XCTAssertFalse(buffer.is_using_stack_buffer_now());
            }
            XCTAssertEqual(dealloc_counter, 0);
        }
        XCTAssertTrue(buffer.size() == loop_count);

        for (size_type idx = 0; idx < loop_count; idx ++) {
            const auto lhs = buffer[idx].getValue();
            const auto rhs = idx;
            XCTAssertEqual(lhs, rhs);
        }
        buffer.erase(buffer.begin(), buffer.end());
        XCTAssertTrue(buffer.empty());
        XCTAssertTrue(buffer.size() == 0);
        XCTAssertFalse(buffer.is_using_stack_buffer_now());
    }
    XCTAssertEqual(dealloc_counter, loop_count);
}

- (void)testStackBasedVector_stack_based_on_heap_insert_element_from_random_position {
    size_t dealloc_counter = 0;
    const size_t stack_buffer_capacity = 0x10;
    using size_type = vector<NotPODTester, stack_buffer_capacity>::size_type;

    const size_type loop_count = 0x20;
    const size_type element_idx = arc4random() % loop_count;
    {
        vector<NotPODTester, stack_buffer_capacity> buffer;
        XCTAssertTrue(buffer.is_stack_buffer_available);
        XCTAssertTrue(buffer.default_alloc_size == stack_buffer_capacity);

        static_assert(loop_count > stack_buffer_capacity,
                      "loop count should be greater than stack_buffer_capacity");

        for (size_type idx = 0; idx < element_idx; idx ++) {
            XCTAssertTrue(buffer.size() == idx);
            buffer.insert(buffer.begin() + idx, {dealloc_counter, idx});
            if (idx < stack_buffer_capacity) {
                XCTAssertTrue(buffer.is_using_stack_buffer_now());
            } else {
                XCTAssertFalse(buffer.is_using_stack_buffer_now());
            }
            XCTAssertEqual(dealloc_counter, 0);
        }

        for (size_type idx = 0; idx < loop_count - element_idx; idx ++) {
            XCTAssertTrue(buffer.size() == element_idx + idx);
            buffer.insert(buffer.begin() + element_idx, {
                dealloc_counter, idx + element_idx
            });
            XCTAssertEqual(dealloc_counter, 0);
        }
        XCTAssertTrue(buffer.size() == loop_count);

        for (size_type idx = 0; idx < element_idx; idx ++) {
            const auto lhs = buffer[idx].getValue();
            const auto rhs = idx;
            XCTAssertEqual(lhs, rhs);
        }

        for (size_type idx = element_idx; idx < loop_count; idx ++) {
            const auto lhs = buffer[idx].getValue();
            const auto rhs = loop_count - (idx - element_idx) - 1;
            XCTAssertEqual(lhs, rhs);
        }
        buffer.erase(buffer.begin(), buffer.end());
        XCTAssertTrue(buffer.empty());
        XCTAssertTrue(buffer.size() == 0);
        XCTAssertFalse(buffer.is_using_stack_buffer_now());
    }
    XCTAssertEqual(dealloc_counter, loop_count);
}

template<typename T, int N>
static void _buffer_isEqualsTo_vector(const vector<T, N> &buffer,
                                      const std::vector<T> &vector) {
    XCTAssertTrue(buffer.size() == vector.size());
    const auto size = buffer.size();
    for (auto idx = 0; idx < size; idx++) {
        assert(buffer[idx] == vector[idx]);
    }
}

template<typename T, int N>
static void _test_buffer(vector<T, N> &buffer,
                         std::vector<T> &std_buffer) {
    size_t dealloc_counter = 0;
    const size_t stack_buffer_capacity = 0x20;
    using size_type = vector<size_t, stack_buffer_capacity>::size_type;

    const size_type loop_count = 0x30;
    {
        vector<size_t, stack_buffer_capacity> buffer;
        XCTAssertTrue(buffer.is_stack_buffer_available);
        XCTAssertTrue(buffer.default_alloc_size == stack_buffer_capacity);

        static_assert(loop_count > stack_buffer_capacity,
                      "loop count should be greater than stack_buffer_capacity");

        std::vector<size_t> std_buffer;

        for (size_type idx = 0; idx < loop_count; idx ++) {
            buffer.push_back({idx});
            std_buffer.push_back({idx});
        }

        _buffer_isEqualsTo_vector<size_t, stack_buffer_capacity>(buffer, std_buffer);

        const auto random_inserted_times = ((arc4random() % loop_count) + 20) % arc4random();
        const auto random_deleted_times = ((arc4random() % loop_count) + 20) % arc4random();


        std::vector<bool> action_buffer;

        {
            size_t inserted_num = 0;
            size_t deleted_num = 0;

            while ((inserted_num < random_inserted_times) && (deleted_num < random_deleted_times)) {
                if (arc4random() % 2 == 1) {
                    action_buffer.push_back(true);
                    inserted_num ++;
                } else {
                    action_buffer.push_back(false);
                    deleted_num ++;
                }
            }

            while (inserted_num < random_inserted_times) {
                action_buffer.push_back(true);
                inserted_num += 1;
            }

            while (deleted_num < random_deleted_times) {
                action_buffer.push_back(false);
                deleted_num += 1;
            }
        }

        XCTAssertTrue(action_buffer.size() == (random_inserted_times + random_deleted_times));

        size_t buf_deleted_count = 0;
        const size_t value_base_idx = loop_count;

        for (size_t action_idx = 0; action_idx < action_buffer.size(); action_idx++) {
            const bool action = action_buffer[action_idx];
            assert(std_buffer.size() == buffer.size());
            const size_t position = arc4random() % std_buffer.size();
            if (action) {
                buffer.insert(buffer.begin() + position, {value_base_idx + position});
                std_buffer.insert(std_buffer.begin() + position, {value_base_idx + position});
                _buffer_isEqualsTo_vector<size_t, stack_buffer_capacity>(buffer, std_buffer);
            } else {
                const auto std_buffer_val = std_buffer[position];
                const auto buffer_val = buffer[position];
                assert(std_buffer_val == std_buffer_val);

                _buffer_isEqualsTo_vector<size_t, stack_buffer_capacity>(buffer, std_buffer);

                auto std_buffer_find_it = std::find(std_buffer.begin(), std_buffer.end(), std_buffer_val);
                const auto std_buffer_find_it_distance = std_buffer_find_it - std_buffer.begin();
                assert(*std_buffer_find_it == std_buffer_val);

                auto buffer_find_it = std::find(buffer.begin(), buffer.end(), buffer_val);
                const auto buffer_find_it_distance = buffer_find_it - buffer.begin();
                assert(*buffer_find_it == buffer_val);

                assert(std_buffer_find_it_distance == buffer_find_it_distance);

                auto std_buffer_remove_it = std::remove(std_buffer.begin(), std_buffer.end(), std_buffer_val);
                assert(std_buffer_remove_it != std_buffer.end());

                auto buffer_remove_it = std::remove(buffer.begin(), buffer.end(), buffer_val);
                assert(buffer_remove_it != buffer.end());

                buffer.erase(buffer_remove_it);
                buf_deleted_count += 1;

                std_buffer.erase(std_buffer_remove_it);

                _buffer_isEqualsTo_vector<size_t, stack_buffer_capacity>(buffer, std_buffer);
            }
        }

        _buffer_isEqualsTo_vector<size_t, stack_buffer_capacity>(buffer, std_buffer);
    }
}

- (void)testStackBasedVector_stack_based_on_heap_insert_and_delete_element_from_random_position {
    size_t dealloc_counter = 0;
    const constexpr size_t stack_buffer_capacity = 0x20;
    using value_type = NotPODTester;
    using size_type = vector<value_type, stack_buffer_capacity>::size_type;

    const size_type loop_count = 0x30;
    {
        vector<value_type, stack_buffer_capacity> buffer;
        XCTAssertTrue(buffer.is_stack_buffer_available);
        XCTAssertTrue(buffer.default_alloc_size == stack_buffer_capacity);

        static_assert(loop_count > stack_buffer_capacity,
                      "loop count should be greater than stack_buffer_capacity");

        std::vector<value_type> std_buffer;

        for (size_type idx = 0; idx < loop_count; idx ++) {
            buffer.push_back({dealloc_counter, idx});
            std_buffer.push_back({dealloc_counter, idx});
        }
        _buffer_isEqualsTo_vector<value_type, stack_buffer_capacity>(buffer, std_buffer);

        const auto random_inserted_times = ((arc4random() % loop_count) + 20) % arc4random();
        const auto random_deleted_times = ((arc4random() % loop_count) + 20) % arc4random();


        std::vector<bool> action_buffer;

        {
            size_t inserted_num = 0;
            size_t deleted_num = 0;

            while ((inserted_num < random_inserted_times) && (deleted_num < random_deleted_times)) {
                if (arc4random() % 2 == 1) {
                    action_buffer.push_back(true);
                    inserted_num ++;
                } else {
                    action_buffer.push_back(false);
                    deleted_num ++;
                }
            }

            while (inserted_num < random_inserted_times) {
                action_buffer.push_back(true);
                inserted_num += 1;
            }

            while (deleted_num < random_deleted_times) {
                action_buffer.push_back(false);
                deleted_num += 1;
            }
        }

        XCTAssertTrue(action_buffer.size() == (random_inserted_times + random_deleted_times));

        size_t buf_deleted_count = 0;
        const size_t value_base_idx = loop_count;

        for (size_t action_idx = 0; action_idx < action_buffer.size(); action_idx++) {
            const bool action = action_buffer[action_idx];
            assert(std_buffer.size() == buffer.size());
            const size_t position = arc4random() % std_buffer.size();
            if (action) {
                buffer.insert(buffer.begin() + position, {dealloc_counter, value_base_idx + position});
                std_buffer.insert(std_buffer.begin() + position, {dealloc_counter, value_base_idx + position});
                _buffer_isEqualsTo_vector<value_type, stack_buffer_capacity>(buffer, std_buffer);
            } else {
                const auto std_buffer_val = std_buffer[position];
                const auto buffer_val = buffer[position];
                assert(std_buffer_val == std_buffer_val);

                _buffer_isEqualsTo_vector<value_type, stack_buffer_capacity>(buffer, std_buffer);

                auto std_buffer_find_it = std::find(std_buffer.begin(), std_buffer.end(), std_buffer_val);
                const auto std_buffer_find_it_distance = std_buffer_find_it - std_buffer.begin();
                assert(*std_buffer_find_it == std_buffer_val);

                auto buffer_find_it = std::find(buffer.begin(), buffer.end(), buffer_val);
                const auto buffer_find_it_distance = buffer_find_it - buffer.begin();
                assert(*buffer_find_it == buffer_val);

                assert(std_buffer_find_it_distance == buffer_find_it_distance);

                auto std_buffer_remove_it = std::remove(std_buffer.begin(), std_buffer.end(), std_buffer_val);
                assert(std_buffer_remove_it != std_buffer.end());

                auto buffer_remove_it = std::remove(buffer.begin(), buffer.end(), buffer_val);
                assert(buffer_remove_it != buffer.end());

                buffer.erase(buffer_remove_it);
                buf_deleted_count += 1;

                std_buffer.erase(std_buffer_remove_it);

                _buffer_isEqualsTo_vector<value_type, stack_buffer_capacity>(buffer, std_buffer);
            }
        }

        _buffer_isEqualsTo_vector<value_type, stack_buffer_capacity>(buffer, std_buffer);
    }
}

- (void)testHeapBasedVector_insert_element_from_begin_position {
    size_t dealloc_counter = 0;
    const size_t buffer_capacity = 0x20;
    using size_type = vector<NotPODTester, 0>::size_type;

    const size_type loop_count = 0x20;
    {
        vector<NotPODTester, 0> buffer;
        XCTAssertFalse(buffer.is_stack_buffer_available);
        XCTAssertEqual(buffer.default_alloc_size, 4);

        static_assert(loop_count == buffer_capacity,
                      "loop count should be equals to stack_buffer_capacity");

        for (size_type idx = 0; idx < loop_count; idx ++) {
            XCTAssertTrue(buffer.size() == idx);
            buffer.insert(buffer.begin(), {dealloc_counter, idx});
            XCTAssertEqual(dealloc_counter, 0);
        }
        XCTAssertTrue(buffer.size() == loop_count);

        for (size_type idx = 0; idx < loop_count; idx ++) {
            const auto lhs = buffer[idx].getValue();
            const auto rhs = loop_count - 1 - idx;
            XCTAssertEqual(lhs, rhs);
        }
        buffer.erase(buffer.begin(), buffer.end());
        XCTAssertTrue(buffer.empty());
        XCTAssertTrue(buffer.size() == 0);
        XCTAssertFalse(buffer.is_using_stack_buffer_now());
    }
    XCTAssertEqual(dealloc_counter, loop_count);
}

- (void)testHeapBasedVector_insert_element_from_end_position {
    size_t dealloc_counter = 0;
    const size_t buffer_capacity = 0x20;
    using size_type = vector<NotPODTester, 0>::size_type;

    const size_type loop_count = 0x20;
    {
        vector<NotPODTester, 0> buffer;
        XCTAssertFalse(buffer.is_stack_buffer_available);
        XCTAssertEqual(buffer.default_alloc_size, 4);

        static_assert(loop_count == buffer_capacity,
                      "loop count should be equals to stack_buffer_capacity");

        for (size_type idx = 0; idx < loop_count; idx ++) {
            XCTAssertTrue(buffer.size() == idx);
            buffer.insert(buffer.end(), {dealloc_counter, idx});
            XCTAssertEqual(dealloc_counter, 0);
        }
        XCTAssertTrue(buffer.size() == loop_count);

        for (size_type idx = 0; idx < loop_count; idx ++) {
            const auto lhs = buffer[idx].getValue();
            const auto rhs = idx;
            XCTAssertEqual(lhs, rhs);
        }
        buffer.erase(buffer.begin(), buffer.end());
        XCTAssertTrue(buffer.empty());
        XCTAssertTrue(buffer.size() == 0);
        XCTAssertFalse(buffer.is_using_stack_buffer_now());
    }
    XCTAssertEqual(dealloc_counter, loop_count);
}

- (void)testHeapBasedVector_insert_element_from_random_position {
    size_t dealloc_counter = 0;
    const size_t buffer_capacity = 0x10;
    using size_type = vector<NotPODTester, 0>::size_type;

    const size_type loop_count = buffer_capacity;
    const size_type element_idx = arc4random() % buffer_capacity;
    {
        vector<NotPODTester, 0> buffer;
        XCTAssertFalse(buffer.is_stack_buffer_available);
        XCTAssertEqual(buffer.default_alloc_size, 4);

        static_assert(loop_count == buffer_capacity,
                      "loop count should be equals to stack_buffer_capacity");

        for (size_type idx = 0; idx < element_idx; idx ++) {
            XCTAssertTrue(buffer.size() == idx);
            buffer.insert(buffer.begin() + idx, {dealloc_counter, idx});
            XCTAssertEqual(dealloc_counter, 0);
        }

        for (size_type idx = 0; idx < loop_count - element_idx; idx ++) {
            XCTAssertTrue(buffer.size() == element_idx + idx);
            buffer.insert(buffer.begin() + element_idx, {
                dealloc_counter, idx + element_idx
            });
            XCTAssertEqual(dealloc_counter, 0);
        }
        XCTAssertTrue(buffer.size() == loop_count);

        for (size_type idx = 0; idx < element_idx; idx ++) {
            const auto lhs = buffer[idx].getValue();
            const auto rhs = idx;
            XCTAssertEqual(lhs, rhs);
        }

        for (size_type idx = element_idx; idx < loop_count; idx ++) {
            const auto lhs = buffer[idx].getValue();
            const auto rhs = loop_count - (idx - element_idx) - 1;
            XCTAssertEqual(lhs, rhs);
        }
        buffer.erase(buffer.begin(), buffer.end());
        XCTAssertTrue(buffer.empty());
        XCTAssertTrue(buffer.size() == 0);
        XCTAssertFalse(buffer.is_using_stack_buffer_now());
    }
    XCTAssertEqual(dealloc_counter, loop_count);
}

- (void)testPerformance_NotPOD_heapOnly {
    [self measureBlock:^{
        size_t dealloc_counter = 0;
        const size_t loop_count = 0x20;
        const size_t repeat_count = 0x10000;

        for (size_t idx = 0; idx < repeat_count; idx ++) {
            vector<NotPODTester, 0> buffer;
            for (size_t idx = 0; idx < loop_count; idx++) {
                buffer.push_back(NotPODTester(dealloc_counter, idx));
                XCTAssertFalse(buffer.is_using_stack_buffer_now());
            }
        }

        XCTAssertTrue(dealloc_counter == repeat_count * loop_count);
    }];
}

- (void)testPerformance_NotPOD_stackOnly {
    [self measureBlock:^{
        size_t dealloc_counter = 0;
        const size_t loop_count = 0x20;
        const size_t repeat_count = 0x10000;

        for (size_t idx = 0; idx < repeat_count; idx ++) {
            vector<NotPODTester, loop_count> buffer;
            for (size_t idx = 0; idx < loop_count; idx++) {
                buffer.push_back(NotPODTester(dealloc_counter, idx));
                XCTAssertTrue(buffer.is_using_stack_buffer_now());
            }
        }

        XCTAssertTrue(dealloc_counter == repeat_count * loop_count);
    }];
}

- (void)testPerformance_POD_heapOnly {
    [self measureBlock:^{
        size_t dealloc_counter = 0;
        const size_t loop_count = 0x200;
        const size_t repeat_count = 0x1000;

        for (size_t idx = 0; idx < repeat_count; idx ++) {
            vector<PODTester, 0> buffer;
            for (size_t idx = 0; idx < loop_count; idx++) {
                buffer.push_back(PODTester(dealloc_counter, idx));
                XCTAssertFalse(buffer.is_using_stack_buffer_now());
            }
        }

        XCTAssertTrue(dealloc_counter == repeat_count * loop_count);
    }];
}

- (void)testPerformance_POD_stackOnly {
    [self measureBlock:^{
        size_t dealloc_counter = 0;
        const size_t loop_count = 0x200;
        const size_t repeat_count = 0x1000;

        for (size_t idx = 0; idx < repeat_count; idx ++) {
            vector<PODTester, loop_count> buffer;
            for (size_t idx = 0; idx < loop_count; idx++) {
                buffer.push_back(PODTester(dealloc_counter, idx));
                XCTAssertTrue(buffer.is_using_stack_buffer_now());
            }
        }

        XCTAssertTrue(dealloc_counter == repeat_count * loop_count);
    }];
}

- (void)testStackBasedVector_contains_stack_based_vector {
    size_t dealloc_counter = 0;
    const size_t stack_buffer_capacity = 0x10;
    using size_type = vector<vector<NotPODTester, stack_buffer_capacity>, stack_buffer_capacity>::size_type;

    const size_type loop_count = stack_buffer_capacity;

    {
        vector<vector<NotPODTester, stack_buffer_capacity>, stack_buffer_capacity> buffer;
        XCTAssertTrue(buffer.is_stack_buffer_available);
        XCTAssertTrue(buffer.default_alloc_size == stack_buffer_capacity);

        for (size_type idx = 0; idx < loop_count; idx ++) {
            vector<NotPODTester, stack_buffer_capacity> inner;
            for (size_type idx = 0; idx < loop_count; idx ++) {
                inner.push_back(NotPODTester(dealloc_counter, idx));
                XCTAssertTrue(inner.is_using_stack_buffer_now());
            }
            buffer.push_back(std::move(inner));
        }

        XCTAssertTrue(buffer.is_using_stack_buffer_now());
    }
    XCTAssertTrue(dealloc_counter == loop_count * loop_count);
}


@end
