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

#ifndef vector_hpp
#define vector_hpp

#include "DanceUI_Debug.hpp"
#include <DanceUIRuntime/DanceUISwiftSupport.h>
#include <DanceUIRuntime/DanceUIRuntimeConfiguration.h>

#include <stddef.h>
#include <memory>
#include <algorithm>
#include <malloc/malloc.h>
#include <boost/iterator.hpp>
#include <assert.h>

namespace DanceUI {

template <typename ValueType, uint32_t stack_buffer_size, typename SizeType>
class vector;

namespace detail {

template <uint32_t element_size, typename SizeType = uint32_t>
void *realloc_vector(void *buffer, SizeType &capacity, SizeType newCapacity) noexcept {
    static_assert(std::is_integral<SizeType>::value, "SizeType should be an integral type");
    if (newCapacity == 0) {
        capacity = 0;
        free(buffer);
        return nullptr;
    }
    const SizeType byte_size = static_cast<const SizeType>(malloc_good_size(newCapacity * element_size));
    const SizeType new_buffer_capacity = byte_size / element_size;
    if (new_buffer_capacity == capacity) {
        return buffer;
    }
    void *new_buffer = realloc(buffer, byte_size);
    if (new_buffer == nullptr) {
        precondition_failure("memory allocation failed, realloc ptr: %p; capacity: %d; new_capacity: %d; new_size: %d;", buffer, capacity, new_buffer_capacity, byte_size);
    }
    capacity = new_buffer_capacity;
    return new_buffer;
}

template <uint32_t element_size, typename SizeType = uint32_t>
void *realloc_vector(void *buffer,
                     void *stack_buffer,
                     SizeType stack_capacity,
                     SizeType &buffer_capacity,
                     SizeType required_capacity) noexcept {
    static_assert(std::is_integral<SizeType>::value, "SizeType should be an integral type");
    if (required_capacity <= stack_capacity) {
        if (buffer == stack_buffer) {
            return stack_buffer;
        }
        memcpy(stack_buffer, buffer, required_capacity * element_size);
        free(buffer);
        buffer_capacity = required_capacity;
        return stack_buffer;
    }
    const SizeType byte_size = static_cast<const SizeType>(malloc_good_size(required_capacity * element_size));
    const SizeType new_buffer_capacity = byte_size / element_size;
    if (buffer_capacity == new_buffer_capacity) {
        return buffer;
    }
    void *new_buffer = nullptr;
    const bool is_using_stack_buffer = buffer == stack_buffer;
    if (is_using_stack_buffer) {
        new_buffer = malloc(byte_size);
    } else {
        new_buffer = realloc(buffer, byte_size);
    }
    if (new_buffer == nullptr) {
        precondition_failure("memory allocation failed");
    }
    if (is_using_stack_buffer) {
        memcpy(new_buffer, stack_buffer, buffer_capacity * element_size);
    }
    buffer_capacity = new_buffer_capacity;
    return new_buffer;
}

template <typename _Iter>
class __wrap_iterator final {
public:

    using iterator_type = _Iter;
    using iterator_category = typename std::iterator_traits<iterator_type>::iterator_category;
    using value_type = typename std::iterator_traits<iterator_type>::value_type;
    using difference_type = typename std::iterator_traits<iterator_type>::difference_type;
    using pointer = typename std::iterator_traits<iterator_type>::pointer;
    using reference = typename std::iterator_traits<iterator_type>::reference;

    DANCE_UI_INLINE constexpr
    __wrap_iterator() noexcept:
    _value() {
    }

    DANCE_UI_INLINE constexpr
    __wrap_iterator(const __wrap_iterator &it) noexcept :
    _value(it._value) {
    }

    template <class _Up>
    DANCE_UI_INLINE constexpr
    __wrap_iterator(const __wrap_iterator<_Up>& __u,
                    typename std::enable_if<std::is_convertible<_Up, iterator_type>::value>::type * = 0) noexcept :
    _value(__u.base()) {
    }

    DANCE_UI_INLINE constexpr
    __wrap_iterator(__wrap_iterator &&it) : _value(std::move(it._value)) {}

    DANCE_UI_INLINE constexpr
    __wrap_iterator &operator=(const __wrap_iterator &it) {
        if (this != &it) {
            _value = it._value;
        }
        return *this;
    }

    DANCE_UI_INLINE constexpr
    __wrap_iterator &operator=(pointer &value) {
        _value = value;
        return *this;
    }

    DANCE_UI_INLINE constexpr
    __wrap_iterator &operator++() {
        ++_value;
        return *this;
    }

    DANCE_UI_INLINE constexpr
    __wrap_iterator &operator--() {
        --_value;
        return *this;
    }

    DANCE_UI_INLINE constexpr
    __wrap_iterator operator++(int) {
        auto __tmp(*this);
        ++(*this);
        return __tmp;
    }

    DANCE_UI_INLINE constexpr
    __wrap_iterator operator--(int) {
        auto __tmp(*this);
        --(*this);
        return __tmp;
    }

    DANCE_UI_INLINE constexpr
    __wrap_iterator operator+(difference_type __n) const noexcept {
        auto __w(*this);
        __w += __n;
        return __w;
    }

    DANCE_UI_INLINE constexpr
    __wrap_iterator &operator+=(difference_type __n) noexcept {
        _value += __n;
        return *this;
    }

    DANCE_UI_INLINE constexpr
    __wrap_iterator operator-(difference_type __n) const noexcept {
        auto __w(*this);
        __w -= __n;
        return __w;
    }

    DANCE_UI_INLINE constexpr
    __wrap_iterator &operator-=(difference_type __n) noexcept {
        _value -= __n;
        return *this;
    }

    DANCE_UI_INLINE constexpr
    difference_type
    operator-(const __wrap_iterator &__position) const noexcept {
        return base() - __position.base();
    }

    DANCE_UI_INLINE constexpr
    reference operator*() const noexcept {
        return *_value;
    }

    DANCE_UI_INLINE constexpr
    pointer operator->() const noexcept {
        return (pointer)std::addressof(*_value);
    }

    DANCE_UI_INLINE constexpr
    reference operator[](difference_type __n) const noexcept {
        return _value[__n];
    }

    DANCE_UI_INLINE constexpr
    bool operator==(const __wrap_iterator &value) const noexcept {
        if (this == &value) {
            return true;
        }
        return _value == value._value;
    }

    DANCE_UI_INLINE constexpr
    bool operator!=(const __wrap_iterator &value) const noexcept {
        return !(*this == value);
    }

    DANCE_UI_INLINE constexpr
    bool operator<(const __wrap_iterator &value) const noexcept {
        if (this == &value) {
            return false;
        }
        return _value < value._value;
    }

    DANCE_UI_INLINE constexpr
    bool operator>(const __wrap_iterator &value) const noexcept {
        if (this == &value) {
            return false;
        }
        return _value > value._value;
    }

    DANCE_UI_INLINE constexpr
    bool operator>=(const __wrap_iterator &value) const noexcept {
        return !(*this < value);
    }

    DANCE_UI_INLINE constexpr
    bool operator<=(const __wrap_iterator &value) const noexcept {
        return !(*this > value);
    }

    DANCE_UI_INLINE constexpr
    iterator_type base() const noexcept {
        return _value;
    }

private:
    DANCE_UI_INLINE constexpr
    __wrap_iterator(iterator_type value) : _value(value) {
    }

    template <typename ValueType, uint32_t stack_buffer_size, typename SizeType>
    friend class DanceUI::vector;

private:
    iterator_type _value;
};

}

template <class _Iter1, class _Iter2>
DANCE_UI_INLINE constexpr
auto
operator-(const DanceUI::detail::__wrap_iterator<_Iter1>& __x,
          const DanceUI::detail::__wrap_iterator<_Iter2>& __y) noexcept
-> decltype(__x.base() - __y.base()) {
    return __x.base() - __y.base();
}

template <class _Cp>
typename _Cp::const_iterator
begin(const _Cp& __c) {
    return __c.begin();
}

template <class _Cp>
typename _Cp::const_iterator
end(const _Cp& __c) {
    return __c.end();
}

template <typename ValueType, uint32_t prealloc_size, typename SizeType = uint32_t>
class vector final {
public:
    static_assert(prealloc_size > 0, "prealloc_size should be over 0!");
    static_assert(std::is_integral<SizeType>::value, "SizeType should be an integral type");

    using allocator_type = std::allocator<ValueType>;
    using __alloc_traits = std::allocator_traits<allocator_type>;
    using size_type = SizeType;

    using value_type = ValueType;
    using reference = value_type &;
    using const_reference = const value_type &;


    using difference_type = typename __alloc_traits::difference_type;
    using pointer = typename __alloc_traits::pointer;
    using const_pointer = typename __alloc_traits::const_pointer;

    using iterator = detail::__wrap_iterator<pointer>;
    using const_iterator = detail::__wrap_iterator<const_pointer>;

    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using reverse_iterator = std::reverse_iterator<iterator>;

    static_assert(sizeof(value_type) > 0, "sizeof(value_type) should be over 0!");

    static constexpr bool is_stack_buffer_available = true;
    static constexpr size_type stack_buffer_byte_size = (prealloc_size * sizeof(value_type) + 15) & (~0xF);
    static constexpr size_type default_alloc_size = stack_buffer_byte_size / sizeof(value_type);

    template <typename Ty, uint32_t M, typename STy>
    friend class vector;

public:

    DANCE_UI_INLINE
    vector() noexcept {
        _common_init();
    }

    DANCE_UI_INLINE
    ~vector() noexcept {
        _destroy_elements();
    }

public:

    DANCE_UI_INLINE
    vector(const vector &instance) noexcept {
        _common_init();
        *this = instance;
    }

    template <size_type U>
    DANCE_UI_INLINE
    vector(const vector<value_type, U> &instance) noexcept {
        _common_init();
        *this = instance;
    }

    DANCE_UI_INLINE
    vector(vector &&instance) noexcept {
        _common_init();
        *this = std::move(instance);
    }

    template <size_type U>
    DANCE_UI_INLINE
    vector(vector<value_type, U> &&instance) noexcept {
        _common_init();
        *this = std::move(instance);
    }

    DANCE_UI_INLINE
    vector &operator=(const vector &instance) noexcept {
        if (reinterpret_cast<const void *>(this) !=
            reinterpret_cast<const void *>(&instance)) {
            assign(instance.begin(), instance.end());


        }
        return *this;
    }

    template <size_type U>
    DANCE_UI_INLINE
    vector &operator=(const vector<value_type, U> &instance) noexcept {
        if (reinterpret_cast<const void *>(this) !=
            reinterpret_cast<const void *>(&instance)) {
            assign(instance.begin(), instance.end());


        }
        return *this;
    }

    DANCE_UI_INLINE
    vector &operator=(vector &&instance) noexcept {
        if (reinterpret_cast<const void *>(this) !=
            reinterpret_cast<const void *>(&instance)) {

            _destroy_elements();


            if (!instance.is_using_stack_buffer_now()) {


                _set_buffer(instance._get_buffer());
                _count = instance._count;
                _capacity = instance._capacity;

                instance._set_buffer(nullptr);
                instance._reset_buffer();
            } else {


                const size_type count = instance.size();
                _grow_if_needed(count);
                for (size_type idx = 0; idx < count; idx ++) {
                    ::new((pointer)(_buffer + idx)) value_type(std::move(instance._buffer[idx]));
                }
                instance._destroy_elements();
                _count = count;
            }
        }
        return *this;
    }

    template <size_type U>
    DANCE_UI_INLINE
    vector &operator=(vector<value_type, U> &&instance) noexcept {
        if (reinterpret_cast<const void *>(this) !=
            reinterpret_cast<const void *>(&instance)) {

            _destroy_elements();


            if (!instance.is_using_stack_buffer_now()) {


                _set_buffer(instance._get_buffer());
                _count = instance._count;
                _capacity = instance._capacity;

                instance._set_buffer(nullptr);
                instance._reset_buffer();
            } else {


                const size_type count = instance.size();
                _grow_if_needed(count);
                for (size_type idx = 0; idx < count; idx ++) {
                    ::new((pointer)(_buffer + idx)) value_type(std::move(instance._buffer[idx]));
                }
                instance._destroy_elements();
                _count = count;
            }
        }
        return *this;
    }

    DANCE_UI_INLINE
    bool operator==(const vector &instance) const noexcept {
        if (reinterpret_cast<const void *>(this) ==
            reinterpret_cast<const void *>(&instance)) {
            return true;
        }
        return size() == instance.size() &&
        std::equal(cbegin(), cend(), instance.cbegin());
    }

    template<size_type U>
    DANCE_UI_INLINE
    bool operator==(const vector<value_type, U> &instance) const noexcept {
        if (reinterpret_cast<const void *>(this) ==
            reinterpret_cast<const void *>(&instance)) {
            return true;
        }
        return size() == instance.size() &&
        std::equal(cbegin(), cend(), instance.cbegin());
    }

    DANCE_UI_INLINE
    void assign(const_iterator begin,
                const_iterator end) noexcept {
        clear();
        for (; begin != end; ++begin) {
            emplace_back(*begin);
        }
    }

    DANCE_UI_INLINE
    void assign(iterator begin,
                iterator end) noexcept {
        clear();
        for (; begin != end; ++begin) {
            emplace_back(std::move(*begin));
        }
    }

    DANCE_UI_INLINE
    void emplace_back(const_reference value) noexcept {
        push_back(value);
    }

    DANCE_UI_INLINE
    void emplace_back(value_type &&value) noexcept {
        push_back(std::move(value));
    }

    template <class... _Args>
    DANCE_UI_INLINE
    void emplace_back(_Args&&... __args) noexcept {
        _grow_if_needed(size() + 1);
        data()[size()] = std::piecewise_construct(__args...);
        _count += 1;
    }

    DANCE_UI_INLINE
    bool empty() const noexcept {
        return size() == 0;
    }

    DANCE_UI_INLINE
    void clear() noexcept {
        if (empty()) {
            return;
        }
        _destroy_elements();
    }

    DANCE_UI_INLINE constexpr
    size_type size() const noexcept {
        return _count;
    }

    DANCE_UI_INLINE constexpr
    size_type capacity() const noexcept {
        return _capacity;
    }

    DANCE_UI_INLINE constexpr
    pointer data() noexcept {
        return _buffer;
    }

    DANCE_UI_INLINE constexpr
    const_pointer data() const noexcept {
        return _buffer;
    }

    DANCE_UI_INLINE constexpr
    reference front() noexcept {
        return data()[0];
    }

    DANCE_UI_INLINE
    const_reference front() const noexcept {
        return data()[0];
    }

    DANCE_UI_INLINE constexpr
    reference back() noexcept {
        return data()[size() - 1];
    }

    DANCE_UI_INLINE constexpr
    const_reference back() const noexcept {
        return data()[size() - 1];
    }

    DANCE_UI_INLINE constexpr
    reference operator[](size_type index) noexcept {
#if defined(__DANCE_UI_RUNTIME_ASSERTION__)
        assert(index < size());
#endif
        return data()[index];
    }

    DANCE_UI_INLINE
    const_reference operator[](size_type index) const noexcept {
#if defined(__DANCE_UI_RUNTIME_ASSERTION__)
        assert(index < size());
#endif
        return data()[index];
    }

    DANCE_UI_INLINE constexpr
    const_iterator begin() const noexcept {
        return __make_iter(data());
    }

    DANCE_UI_INLINE constexpr
    const_iterator end() const noexcept {
        return __make_iter(data() + size());
    }

    DANCE_UI_INLINE constexpr
    const_iterator cbegin() const noexcept {
        return begin();
    }

    DANCE_UI_INLINE constexpr
    const_iterator cend() const noexcept {
        return end();
    }

    DANCE_UI_INLINE constexpr
    iterator begin() noexcept {
        return __make_iter(data());
    }

    DANCE_UI_INLINE constexpr
    iterator end() noexcept {
        return __make_iter(data() + size());
    }

    DANCE_UI_INLINE constexpr
    reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }

    DANCE_UI_INLINE constexpr
    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    DANCE_UI_INLINE constexpr
    reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }

    DANCE_UI_INLINE constexpr
    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    DANCE_UI_INLINE constexpr
    void push_back(const_reference value) noexcept {
        _grow_if_needed(size() + 1);
        new((pointer)(_get_buffer() + size())) value_type(value);
        _count += 1;
    }

    DANCE_UI_INLINE
    void push_back(value_type &&value) noexcept {
        _grow_if_needed(size() + 1);
        ::new((pointer)(_get_buffer() + size())) value_type(std::move(value));
        _count += 1;
    }

    DANCE_UI_INLINE constexpr
    iterator insert(const_iterator position,
                    const_reference value) noexcept {
        if (empty() || position == cend()) {
            push_back(std::move(value));
            return end();
        }
        const difference_type distance = position - begin();
        _grow_if_needed(size() + 1);
        pointer p = data() + distance;
        __move_range(p, data() + size(), p + 1);
        *p = value;
        _count += 1;
        return begin() + distance;
    }

    DANCE_UI_INLINE constexpr
    iterator insert(const_iterator position,
                    value_type &&value) noexcept {
        if (empty() || position == cend()) {
            push_back(std::move(value));
            return end();
        }
        const difference_type distance = position - begin();
        _grow_if_needed(size() + 1);
        pointer p = data() + distance;
        __move_range(p, data() + size(), p + 1);
        *p = std::move(value);
        _count += 1;
        return begin() + distance;
    }

    DANCE_UI_INLINE
    void pop_back() noexcept {
        if (empty()) {
            return;
        }
        const auto size = _count;
        _get_buffer()[size - 1].~value_type();
        _count -= 1;
    }

    DANCE_UI_INLINE
    bool is_using_stack_buffer_now() const noexcept {
        return reinterpret_cast<const void *>(_get_buffer()) ==
        reinterpret_cast<const void *>(_stack_buffer);
    }

    DANCE_UI_INLINE constexpr
    void reserve(size_type required_capacity) noexcept {
        _grow_if_needed(required_capacity);
    }

    DANCE_UI_INLINE constexpr
    iterator erase(const_iterator __position) noexcept {
        difference_type __ps = __position - begin();
        pointer __p = data() + __ps;
        __destruct_at_end(std::move(__p + 1, data() + size(), __p));
        iterator __r = __make_iter(__p);
        return __r;
    }

    DANCE_UI_INLINE constexpr
    iterator erase(const_iterator __first, const_iterator __last) noexcept {
#if defined(__DANCE_UI_RUNTIME_ASSERTION__)
        assert(__first <= __last && "vector::erase(first, last) called with invalid range");
#endif
        pointer __p = data() + (__first - begin());
        if (__first != __last) {
            __destruct_at_end(std::move(__p + (__last - __first), data() + size(), __p));
        }
        iterator __r = __make_iter(__p);
        return __r;
    }

private:

    DANCE_UI_INLINE
    iterator
    __make_iter(pointer p) noexcept {
        return iterator(p);
    }

    DANCE_UI_INLINE
    const_iterator
    __make_iter(const_pointer p) const noexcept {
        return const_iterator(p);
    }

    DANCE_UI_INLINE
    void _grow_if_needed(size_type required_capacity) noexcept {
        if (required_capacity <= capacity()) {
            return;
        }

        required_capacity = std::max<size_type>(capacity() * 1.5, required_capacity);
        _set_buffer(
        reinterpret_cast<value_type *>(detail::realloc_vector<sizeof(value_type)>
                                       (reinterpret_cast<void *>(_get_buffer()),
                                        reinterpret_cast<void *>(_stack_buffer),
                                        default_alloc_size,
                                        _capacity,
                                        required_capacity)));
    }

    DANCE_UI_INLINE
    void __destruct_at_end(const_iterator __position) noexcept {
        const difference_type __ps = __position - begin();
        pointer __soon_to_be_end = data() + size();
        pointer __new_last = data() + __ps;
        allocator_type alloc;
        while (__new_last != __soon_to_be_end) {
            __alloc_traits::destroy(alloc, std::__to_address(--__soon_to_be_end));
        }
        _count = static_cast<size_type>(__ps);
    }

    DANCE_UI_INLINE
    void _destroy_elements() noexcept {
        __destruct_at_end(cbegin());
        _reset_buffer();
    }

    DANCE_UI_INLINE
    void _reset_buffer() noexcept {
        if (!is_using_stack_buffer_now()) {
            free(_get_buffer());
        }
        _common_init();
    }

    DANCE_UI_INLINE constexpr
    void __move_range(pointer from_start,
                      pointer from_end,
                      pointer to) noexcept {
        pointer old_last = data() + size();
        const difference_type distance = std::distance(to, old_last);

        pointer idx = from_start + distance;
        pointer pos = data() + size();

        allocator_type allocator;
        for (; idx < from_end; ++idx, ++pos) {
            __alloc_traits::construct(allocator,
                                      pos,
                                      std::move(*idx));
        }
        std::move_backward(from_start, from_start + distance, old_last);
    }

    DANCE_UI_INLINE
    void _set_buffer(pointer ptr) noexcept {
        _buffer = ptr;
    }

    DANCE_UI_INLINE
    pointer _get_buffer() noexcept {
        return _buffer;
    }

    DANCE_UI_INLINE
    const_pointer _get_buffer() const noexcept {
        return _buffer;
    }

    DANCE_UI_INLINE
    void _common_init() noexcept {
        _set_buffer(reinterpret_cast<pointer>(_stack_buffer));
        _count = 0;
        _capacity = default_alloc_size;
    }
private:
    uint8_t _stack_buffer[stack_buffer_byte_size] = {0};
    pointer _buffer;
    size_type _count = 0;
    size_type _capacity = 0;
};


template <typename ValueType, typename SizeType>
class vector<ValueType, 0, SizeType> final {
public:

    static_assert(std::is_integral<SizeType>::value, "SizeType should be an integral type");

    template <typename T, uint32_t M, typename S>
    friend class vector;

    using allocator_type = std::allocator<ValueType>;
    using __alloc_traits = std::allocator_traits<allocator_type>;
    using size_type = SizeType;

    using value_type = ValueType;
    using reference = value_type &;
    using const_reference = const value_type &;


    using difference_type = typename __alloc_traits::difference_type;
    using pointer = typename __alloc_traits::pointer;
    using const_pointer = typename __alloc_traits::const_pointer;

    using iterator = detail::__wrap_iterator<pointer>;
    using const_iterator = detail::__wrap_iterator<const_pointer>;

    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using reverse_iterator = std::reverse_iterator<iterator>;

    static constexpr bool is_stack_buffer_available = false;
    static constexpr size_type default_alloc_size = 4;
    static constexpr size_type stack_buffer_byte_size = 0;

public:

    DANCE_UI_INLINE constexpr
    vector() noexcept {
        _buffer = reinterpret_cast<pointer>(malloc(sizeof(value_type) * default_alloc_size));
        memset(_buffer, 0, sizeof(value_type) * default_alloc_size);
        _count = 0;
        _capacity = default_alloc_size;
    }

    DANCE_UI_INLINE
    ~vector() noexcept {
        _destroy_elements();
    }

    DANCE_UI_INLINE constexpr
    vector(const vector &instance) noexcept {
        _common_init();
        *this = instance;
    }

    template<size_t U>
    DANCE_UI_INLINE constexpr
    vector(const vector<value_type, U> &instance) noexcept {
        _common_init();
        *this = instance;
    }

    DANCE_UI_INLINE constexpr
    vector(vector &&instance) noexcept {
        _common_init();
        *this = std::move(instance);
    }

    template<size_t U>
    DANCE_UI_INLINE constexpr
    vector(vector<value_type, U> &&instance) noexcept {
        _common_init();
        *this = std::move(instance);
    }

    DANCE_UI_INLINE
    vector &operator=(const vector &instance) noexcept {
        if (reinterpret_cast<const void *>(this) !=
            reinterpret_cast<const void *>(&instance)) {
            assign(instance.begin(), instance.end());


        }
        return *this;
    }

    template<size_type U>
    DANCE_UI_INLINE
    vector &operator=(const vector<value_type, U> &instance) noexcept {
        if (reinterpret_cast<const void *>(this) !=
            reinterpret_cast<const void *>(&instance)) {
            assign(instance.begin(), instance.end());


        }
        return *this;
    }

    DANCE_UI_INLINE
    vector &operator=(vector &&instance) noexcept {
        if (reinterpret_cast<const void *>(this) !=
            reinterpret_cast<const void *>(&instance)) {

            _destroy_elements();


            if (!instance.is_using_stack_buffer_now()) {


                _set_buffer(instance._get_buffer());
                _count = instance._count;
                _capacity = instance._capacity;

                instance._set_buffer(nullptr);
                instance._reset_buffer();
            } else {


                const size_type count = instance.size();
                _grow_if_needed(count);
                for (size_type idx = 0; idx < count; idx ++) {
                    ::new((pointer)(_get_buffer() + idx)) value_type(std::move(instance._get_buffer()[idx]));
                }
                instance._destroy_elements();
                _count = count;
            }
        }
        return *this;
    }

    template <size_type U>
    DANCE_UI_INLINE
    vector &operator=(vector<value_type, U> &&instance) noexcept {
        if (reinterpret_cast<const void *>(this) !=
            reinterpret_cast<const void *>(&instance)) {

            _destroy_elements();


            if (!instance.is_using_stack_buffer_now()) {


                _set_buffer(instance._get_buffer());
                _count = instance._count;
                _capacity = instance._capacity;

                instance._set_buffer(nullptr);
                instance._reset_buffer();
            } else {


                const size_type count = instance.size();
                _grow_if_needed(count);
                for (size_type idx = 0; idx < count; idx ++) {
                    ::new((pointer)(_get_buffer() + idx)) value_type(std::move(instance._get_buffer()[idx]));
                }
                instance._destroy_elements();
                _count = count;
            }
        }
        return *this;
    }

    DANCE_UI_INLINE
    bool operator==(const vector &instance) const noexcept {
        if (reinterpret_cast<const void *>(this) ==
            reinterpret_cast<const void *>(&instance)) {
            return true;
        }
        return size() == instance.size() &&
        std::equal(begin(), end(), instance.begin());
    }

    template <size_type U>
    DANCE_UI_INLINE
    bool operator==(const vector<value_type, U> &instance) const noexcept {
        if (reinterpret_cast<const void *>(this) ==
            reinterpret_cast<const void *>(&instance)) {
            return true;
        }
        return size() == instance.size() &&
        std::equal(begin(), end(), instance.begin());
    }

    DANCE_UI_INLINE constexpr
    void assign(const_iterator begin,
                const_iterator end) noexcept {
        clear();
        for (; begin != end; ++begin) {
            emplace_back(*begin);
        }
    }

    DANCE_UI_INLINE constexpr
    void assign(iterator begin,
                iterator end) noexcept {
        clear();
        for (; begin != end; ++begin) {
            emplace_back(std::move(*begin));
        }
    }

    DANCE_UI_INLINE constexpr
    void emplace_back(const_reference value) noexcept {
        push_back(value);
    }

    DANCE_UI_INLINE constexpr
    void emplace_back(value_type &&value) noexcept {
        push_back(std::move(value));
    }

    template <class... _Args>
    DANCE_UI_INLINE constexpr
    void emplace_back(_Args&&... __args) noexcept {
        _grow_if_needed(size() + 1);
        data()[size()] = std::piecewise_construct(__args...);
        _count += 1;
    }

    DANCE_UI_INLINE constexpr
    bool empty() const noexcept {
        return size() == 0;
    }

    DANCE_UI_INLINE constexpr
    void clear() noexcept {
        if (empty()) {
            return;
        }
        _destroy_elements();
    }

    DANCE_UI_INLINE constexpr
    const size_type &size() const noexcept {
        return _count;
    }

    DANCE_UI_INLINE constexpr
    size_type capacity() const noexcept {
        return _capacity;
    }

    DANCE_UI_INLINE constexpr
    pointer data() noexcept {
        return _get_buffer();
    }

    DANCE_UI_INLINE constexpr
    const_pointer data() const noexcept {
        return _get_buffer();
    }

    DANCE_UI_INLINE constexpr
    reference front() noexcept {
#if defined(__DANCE_UI_RUNTIME_ASSERTION__)
        assert(size() > 0);
#endif
        return data()[0];
    }

    DANCE_UI_INLINE constexpr
    const_reference front() const noexcept {
#if defined(__DANCE_UI_RUNTIME_ASSERTION__)
        assert(size() > 0);
#endif
        return data()[0];
    }

    DANCE_UI_INLINE constexpr
    reference back() noexcept {
#if defined(__DANCE_UI_RUNTIME_ASSERTION__)
        assert(size() > 0);
#endif
        return data()[size() - 1];
    }

    DANCE_UI_INLINE constexpr
    const_reference back() const noexcept {
#if defined(__DANCE_UI_RUNTIME_ASSERTION__)
        assert(size() > 0);
#endif
        return data()[size() - 1];
    }

    DANCE_UI_INLINE constexpr
    reference operator[](size_type index) noexcept {
#if defined(__DANCE_UI_RUNTIME_ASSERTION__)
        assert(index < size());
#endif
        return data()[index];
    }

    DANCE_UI_INLINE constexpr
    const_reference operator[](size_type index) const noexcept {
#if defined(__DANCE_UI_RUNTIME_ASSERTION__)
        assert(index < size());
#endif
        return data()[index];
    }

    DANCE_UI_INLINE constexpr
    const_iterator begin() const noexcept {
        return __make_iter(data());
    }

    DANCE_UI_INLINE constexpr
    const_iterator end() const noexcept {
        return __make_iter(data() + size());
    }

    DANCE_UI_INLINE constexpr
    const_iterator cbegin() const noexcept {
        return begin();
    }

    DANCE_UI_INLINE constexpr
    const_iterator cend() const noexcept {
        return end();
    }

    DANCE_UI_INLINE constexpr
    iterator begin() noexcept {
        return __make_iter(data());
    }

    DANCE_UI_INLINE constexpr
    iterator end() noexcept {
        return __make_iter(data() + size());
    }

    DANCE_UI_INLINE constexpr
    reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }

    DANCE_UI_INLINE constexpr
    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    DANCE_UI_INLINE constexpr
    reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }

    DANCE_UI_INLINE constexpr
    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    DANCE_UI_INLINE constexpr
    void push_back(const_reference value) noexcept {
        _grow_if_needed(size() + 1);
        new((pointer)(_get_buffer() + size())) value_type(value);
        _count += 1;
    }

    DANCE_UI_INLINE constexpr
    void push_back(value_type &&value) noexcept {
        _grow_if_needed(size() + 1);
        ::new((pointer)(_get_buffer() + size())) value_type(std::move(value));
        _count += 1;
    }

    DANCE_UI_INLINE constexpr
    iterator insert(const_iterator position,
                    const_reference value) noexcept {
        if (empty() || position == cend()) {
            push_back(std::move(value));
            return end();
        }
        const difference_type distance = position - begin();
        _grow_if_needed(size() + 1);
        pointer p = data() + distance;
        __move_range(p, data() + size(), p + 1);
        *p = value;
        _count += 1;
        return begin() + distance;
    }

    DANCE_UI_INLINE constexpr
    iterator insert(const_iterator position,
                    value_type &&value) noexcept {
        if (empty() || position == cend()) {
            push_back(std::move(value));
            return end();
        }
        const difference_type distance = position - begin();
        _grow_if_needed(size() + 1);
        pointer p = data() + distance;
        __move_range(p, data() + size(), p + 1);
        *p = std::move(value);
        _count += 1;
        return begin() + distance;
    }

    DANCE_UI_INLINE constexpr
    void pop_back() noexcept {
        if (empty()) {
            return;
        }
        const auto size = _count;
        _get_buffer()[size - 1].~value_type();
        _count -= 1;
    }

    DANCE_UI_INLINE constexpr
    bool is_using_stack_buffer_now() const noexcept {
        return false;
    }

    DANCE_UI_INLINE constexpr
    void reserve(size_type required_capacity) noexcept {
        _grow_if_needed(required_capacity);
    }

    DANCE_UI_INLINE constexpr
    iterator erase(const_iterator __position) noexcept {
        const difference_type __ps = __position - begin();
        pointer __p = data() + __ps;
        this->__destruct_at_end(std::move(__p + 1, data() + size(), __p));
        iterator __r = __make_iter(__p);
        return __r;
    }

    DANCE_UI_INLINE constexpr
    iterator erase(const_iterator __first, const_iterator __last) noexcept {
#if defined(__DANCE_UI_RUNTIME_ASSERTION__)
        assert(__first <= __last && "vector::erase(first, last) called with invalid range");
#endif
        pointer __p = data() + (__first - begin());
        if (__first != __last) {
            this->__destruct_at_end(std::move(__p + (__last - __first), data() + size(), __p));
        }
        iterator __r = __make_iter(__p);
        return __r;
    }

private:

    DANCE_UI_INLINE constexpr
    iterator
    __make_iter(pointer p) noexcept {
        return iterator(p);
    }

    DANCE_UI_INLINE constexpr
    const_iterator
    __make_iter(const_pointer p) const noexcept {
        return const_iterator(p);
    }

    DANCE_UI_INLINE
    void _grow_if_needed(size_type required_capacity) noexcept {
        if (required_capacity <= capacity()) {
            return;
        }
        required_capacity = std::max<size_type>(capacity() * 1.5, required_capacity);
        _set_buffer(
        reinterpret_cast<value_type *>(detail::realloc_vector<sizeof(value_type)>
                                       (reinterpret_cast<void *>(_get_buffer()),
                                        _capacity,
                                        required_capacity)));
    }

    DANCE_UI_INLINE
    void __destruct_at_end(const_iterator __position) noexcept {
        const difference_type __ps = __position - begin();
        pointer __soon_to_be_end = data() + size();
        pointer __new_last = data() + __ps;
        allocator_type alloc;
        while (__new_last != __soon_to_be_end) {
            __alloc_traits::destroy(alloc, std::__to_address(--__soon_to_be_end));
        }
        _count = static_cast<size_type>(__ps);
    }

    DANCE_UI_INLINE constexpr
    void _destroy_elements() noexcept {
        if (_get_buffer() == nullptr) {
            return;
        }
        __destruct_at_end(cbegin());
        _reset_buffer();
    }

    DANCE_UI_INLINE constexpr
    void _reset_buffer() noexcept {
        if (_get_buffer() != nullptr) {
            free(_get_buffer());
        }
        _set_buffer(nullptr);
        _count = 0;
        _capacity = 0;
    }

    DANCE_UI_INLINE constexpr
    void __move_range(pointer from_start,
                      pointer from_end,
                      pointer to) noexcept {
        pointer old_last = data() + size();
        const difference_type distance = std::distance(to, old_last);

        pointer idx = from_start + distance;
        pointer pos = data() + size();

        allocator_type allocator;
        for (; idx < from_end; ++idx, ++pos) {
            __alloc_traits::construct(allocator,
                                      pos,
                                      std::move(*idx));
        }
        std::move_backward(from_start, from_start + distance, old_last);
    }

    DANCE_UI_INLINE
    const_pointer _get_buffer() const noexcept {
        return _buffer;
    }

    DANCE_UI_INLINE
    pointer _get_buffer() noexcept {
        return _buffer;
    }

    DANCE_UI_INLINE
    void _set_buffer(pointer ptr) noexcept {
        _buffer = ptr;
    }

    DANCE_UI_INLINE
    void _common_init() noexcept {
        _set_buffer(nullptr);
        _count = 0;
        _capacity = 0;
    }

private:

    pointer _buffer = nullptr;
    size_type _count = 0;
    size_type _capacity = 0;
};

}

#endif
