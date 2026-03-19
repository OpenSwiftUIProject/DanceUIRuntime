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

#ifndef LayoutDescriptor_hpp
#define LayoutDescriptor_hpp

#include <swift/Runtime/Metadata.h>

#include <DanceUIRuntime/metadata_visitor.hpp>
#include <DanceUIRuntime/vector.hpp>
#include <DanceUIRuntime/DanceUIComparisonMode.h>

#include <mutex>
#include <unordered_map>

#include <boost/variant.hpp>

using namespace swift;

namespace DanceUI {

bool compare_bytes(const void *lhs, const void *rhs, size_t length) noexcept;

DANCE_UI_INLINE
DanceUIComparisonMode DanceUIComparisonModeGetComparison(DanceUIComparisonMode mode) {
    return mode & DanceUIComparisonModeEquatable;
}

DANCE_UI_INLINE
bool can_use_equatable(const DanceUIComparisonMode suggested_options,
                       const DanceUIComparisonMode derived_options) noexcept {
    return DanceUIComparisonModeGetComparison(derived_options) <= DanceUIComparisonModeGetComparison(suggested_options);
}

class LayoutDescriptor {
public:

    enum class HeapMode {
        HeapMode_0 = 0,
        HeapMode_1 = 1,
        HeapMode_2 = 2,
    };

    enum class HeapType {
        Unknwon = 0x0,
        Class = 0x1,
        LocalVariable = 0x2,
        GenericLocalVariable = 0x4,
    };

    static void print(const char *buffer);

    static size_t length(const char *buffer);

    static DanceUIComparisonMode mode_for_type(const Swift::metadata *metadata,
                                               DanceUIComparisonMode comparison_mode);

    DANCE_UI_INLINE
    const Metadata *get_type() const noexcept {
        return _type;
    }

    class Builder: public Swift::metadata_visitor {
    public:
        enum : uint8_t {
            ActionReadMask = 0x80,
            ActionSkipMask = 0x40,
        };

        enum : uint8_t {
            ActionReadValueMask = 0x7F,
            ActionSkipValueMask = 0x3F,
        };

        enum class Type: uintptr_t {
            CloseExpression = 0,
            Equatable = 1,
            Indirect = 2,
            Existential = 3,
            HeapRef = 4,
            CaptureRef = 5,
            Type_0x6 = 6,
            Enum = 7,
            Enum_0x8 = 8,
            Enum_0x9 = 9,
            Type_0xA = 0xA,

            Case_0 = 0xB,
            Case_1 = 0xC,
            Case_2 = 0xD,
            Case_3 = 0xE,
            Case_4 = 0xF,
            Case_5 = 0x10,
            Case_6 = 0x11,
            Case_7 = 0x12,
            Case_8 = 0x13,

            CloseExpressionTwice = 0x14,

            Count = CloseExpressionTwice,
        };

        struct DataItem;
        struct EqualsItem;
        struct IndirectItem;
        struct ExistentialItem;
        struct HeapRefItem;
        struct EnumItem;
        struct NestedItem;

        using ElementType = boost::variant<
        DataItem,
        EqualsItem,
        IndirectItem,
        ExistentialItem,
        HeapRefItem,
        EnumItem,
        NestedItem>;

        using ElementTypeBufferType = vector<Builder::ElementType, 0, uint32_t>;
        using LayoutBufferType = vector<Builder::ElementType, 16, uint32_t>;

        struct RangeItem {

            DANCE_UI_INLINE constexpr
            RangeItem(uintptr_t start_offset = 0,
                      uintptr_t instance_size = 0,
                      uintptr_t skip_size = 0) noexcept :
            start_offset(start_offset),
            instance_size(instance_size),
            skip_size(skip_size) {
            }

            DANCE_UI_INLINE constexpr
            RangeItem(const RangeItem &value) noexcept :
            start_offset(0),
            instance_size(0),
            skip_size(0) {
                *this = value;
            }

            DANCE_UI_INLINE constexpr
            RangeItem &operator=(const RangeItem &value) noexcept {
                if (this != &value) {
                    start_offset = value.start_offset;
                    instance_size = value.instance_size;
                    skip_size = value.skip_size;
                }
                return *this;
            }

            DANCE_UI_INLINE constexpr
            RangeItem(RangeItem &&value) noexcept :
            start_offset(0),
            instance_size(0),
            skip_size(0) {
                *this = std::move(value);
            }

            DANCE_UI_INLINE constexpr
            RangeItem &operator=(RangeItem &&value) noexcept {
                if (this != &value) {
                    start_offset = value.start_offset;
                    instance_size = value.instance_size;
                    skip_size = value.skip_size;
                }
                return *this;
            }

            DANCE_UI_INLINE constexpr
            bool operator==(const RangeItem &value) const noexcept {
                return start_offset == value.start_offset &&
                instance_size == value.instance_size &&
                skip_size == value.skip_size;
            }


            uintptr_t start_offset;

            uintptr_t instance_size;
            uintptr_t skip_size;
        };


        struct DataItem: RangeItem {

            DANCE_UI_INLINE constexpr
            DataItem(uintptr_t start_offset = 0,
                     uintptr_t instance_size = 0,
                     uintptr_t stride_size = 0,
                     bool skip = false) noexcept :
            RangeItem(start_offset, instance_size, stride_size),
            skip(skip) {

            }

            DataItem(const DataItem &) = delete;

            DANCE_UI_INLINE constexpr
            DataItem(DataItem &&value) noexcept :
            skip(false) {
                *this = std::move(value);
            }

            DANCE_UI_INLINE constexpr
            DataItem &operator=(DataItem &&value) noexcept {
                if (this != &value) {
                    RangeItem::operator=(std::move(value));
                    skip = value.skip;
                }
                return *this;
            }

            DANCE_UI_INLINE constexpr
            bool operator==(const DataItem &value) const noexcept {
                return RangeItem::operator==(value) && skip == value.skip;
            }

            bool skip;
        };


        struct EqualsItem: RangeItem {
            DANCE_UI_INLINE constexpr
            EqualsItem(uintptr_t start_offset = 0,
                       uintptr_t instance_size = 0,
                       uintptr_t skip_size = 0,
                       const Metadata *type = nullptr,
                       WitnessTablePointer equatable = nullptr) noexcept :
            RangeItem(start_offset, instance_size, skip_size),
            type(type),
            equatable(equatable) {}

            EqualsItem(const EqualsItem &) = delete;

            DANCE_UI_INLINE constexpr
            EqualsItem(EqualsItem &&value) noexcept :
            type(nullptr),
            equatable(nullptr) {
                *this = std::move(value);
            }

            DANCE_UI_INLINE constexpr
            EqualsItem &operator=(EqualsItem &&value) noexcept {
                if (this != &value) {
                    RangeItem::operator=(std::move(value));
                    type = value.type;
                    equatable = value.equatable;
                }
                return *this;
            }

            DANCE_UI_INLINE constexpr
            bool operator==(const EqualsItem &value) const noexcept {
                return RangeItem::operator==(value) &&
                type == value.type &&
                equatable == value.equatable;
            }


            const Metadata *type;

            WitnessTablePointer equatable;

            const uintptr_t reserved[3] = {0};

            const Type kind = Type::Equatable;
        };


        struct IndirectItem: RangeItem {

            DANCE_UI_INLINE constexpr
            IndirectItem(uintptr_t start_offset = 0,
                         uintptr_t instance_size = 0,
                         uintptr_t stride_size = 0,
                         const Metadata *type = nullptr) noexcept :
            RangeItem(start_offset, instance_size, stride_size), type(type) {

            }

            IndirectItem(const IndirectItem &value) = delete;

            DANCE_UI_INLINE constexpr
            IndirectItem(IndirectItem &&value) noexcept :
            type(nullptr) {
                *this = std::move(value);
            }

            DANCE_UI_INLINE constexpr
            IndirectItem &operator=(IndirectItem &&value) noexcept {
                if (this != &value) {
                    RangeItem::operator=(std::move(value));
                    type = value.type;
                }
                return *this;
            }

            DANCE_UI_INLINE constexpr
            bool operator==(const IndirectItem &value) const noexcept {
                return RangeItem::operator==(value) &&
                type == value.type;
            }


            const Metadata *type;
        };


        struct ExistentialItem: RangeItem {

            DANCE_UI_INLINE constexpr
            ExistentialItem(uintptr_t start_offset = 0,
                            uintptr_t instance_size = 0,
                            uintptr_t stride_size = 0,
                            const Metadata *type = nullptr) noexcept :
            RangeItem(start_offset, instance_size, stride_size), type(type) {

            }

            ExistentialItem(const ExistentialItem &value) = delete;

            DANCE_UI_INLINE constexpr
            ExistentialItem(ExistentialItem &&value) noexcept :
            type(nullptr) {
                *this = std::move(value);
            }

            DANCE_UI_INLINE constexpr
            ExistentialItem &operator=(ExistentialItem &&value) noexcept {
                if (this != &value) {
                    RangeItem::operator=(std::move(value));
                    type = value.type;
                }
                return *this;
            }

            DANCE_UI_INLINE constexpr
            bool operator==(const ExistentialItem &value) const noexcept {
                return RangeItem::operator==(value) &&
                type == value.type;
            }


            const Metadata *type;

            uintptr_t reserved[4] = {0};

        };


        struct HeapRefItem: RangeItem {

            DANCE_UI_INLINE constexpr
            HeapRefItem(uintptr_t start_offset = 0,
                        uintptr_t instance_size = 0,
                        uintptr_t skip_size = 0,
                        bool isHeapLocalVariable = false) noexcept :
            RangeItem(start_offset, instance_size, skip_size),
            isHeapLocalVariable(isHeapLocalVariable) {
            }

            HeapRefItem(const HeapRefItem &value) = delete;

            DANCE_UI_INLINE constexpr
            HeapRefItem(HeapRefItem &&value) noexcept :
            isHeapLocalVariable(false) {
                *this = std::move(value);
            }

            DANCE_UI_INLINE constexpr
            HeapRefItem &operator=(HeapRefItem &&value) noexcept {
                if (this != &value) {
                    RangeItem::operator=(std::move(value));
                    isHeapLocalVariable = value.isHeapLocalVariable;
                }
                return *this;
            }

            DANCE_UI_INLINE constexpr
            bool operator==(const HeapRefItem &value) const noexcept {
                return RangeItem::operator==(value) &&
                isHeapLocalVariable == value.isHeapLocalVariable;
            }

            bool isHeapLocalVariable;

            uintptr_t reserved[4] = {0};
        };

        struct ReferenceItem;


        struct EnumItem: RangeItem {

            DANCE_UI_INLINE
            EnumItem(uintptr_t start_offset = 0,
                     uintptr_t instance_size = 0,
                     uintptr_t stride_size = 0,
                     const Metadata *type = nullptr) noexcept :
            RangeItem(start_offset, instance_size, stride_size), type(type) {

            }

            EnumItem(const EnumItem &value) = delete;

            DANCE_UI_INLINE
            EnumItem(EnumItem &&value) noexcept {
                *this = std::move(value);
            }

            DANCE_UI_INLINE constexpr
            EnumItem &operator=(EnumItem &&value) noexcept {
                if (this != &value) {
                    RangeItem::operator=(std::move(value));
                    type = std::move(value.type);
                    cases = std::move(value.cases);
                }
                return *this;
            }

            DANCE_UI_INLINE constexpr
            bool operator==(const EnumItem &value) const noexcept {
                return RangeItem::operator==(value) &&
                type == value.type &&
                cases == value.cases;
            }


            const Metadata *type;

            vector<ReferenceItem, 0> cases;
        };


        struct ReferenceItem {
            size_t offset = 0;
            size_t bufferPosition = 0;

            ElementTypeBufferType buffer;

            DANCE_UI_INLINE
            ReferenceItem(size_t offset,
                          size_t bufferPosition) noexcept :
            offset(offset), bufferPosition(bufferPosition) {
            }

            ReferenceItem(const ReferenceItem &) = delete;

            DANCE_UI_INLINE
            ReferenceItem(ReferenceItem &&value) noexcept {
                *this = std::move(value);
            }

            DANCE_UI_INLINE constexpr
            ReferenceItem &operator=(ReferenceItem &&value) noexcept {
                if (this != &value) {
                    offset = value.offset;
                    bufferPosition = value.bufferPosition;
                    buffer = std::move(value.buffer);
                }
                return *this;
            }

            DANCE_UI_INLINE constexpr
            bool operator==(const ReferenceItem &value) const noexcept {
                return offset == value.offset &&
                bufferPosition == value.bufferPosition &&
                buffer == value.buffer;
            }
        };

        struct NestedItem: RangeItem {

            DANCE_UI_INLINE
            NestedItem(uintptr_t start_offset,
                       uintptr_t instance_size,
                       uintptr_t stride_size,
                       const LayoutDescriptor *layout_descriptor) noexcept :
            RangeItem(start_offset, instance_size, stride_size), layout_descriptor(layout_descriptor) {
            }

            NestedItem(const NestedItem &value) = delete;

            DANCE_UI_INLINE
            NestedItem(NestedItem &&value) noexcept : layout_descriptor(value.layout_descriptor) {
                *this = std::move(value);
            }

            DANCE_UI_INLINE constexpr
            NestedItem &operator=(NestedItem &&value) noexcept {
                if (this != &value) {
                    RangeItem::operator=(std::move(value));
                    layout_descriptor = std::move(value.layout_descriptor);
                }
                return *this;
            }

            DANCE_UI_INLINE constexpr
            bool operator==(const NestedItem &value) const noexcept {
                return RangeItem::operator==(value) &&
                layout_descriptor == value.layout_descriptor;
            }

            DANCE_UI_INLINE
            const Metadata *get_type() const noexcept {
                return layout_descriptor->get_type();
            }


            const LayoutDescriptor *layout_descriptor;
        };

        template <typename Container>
        class Emitter {
        public:

            DANCE_UI_INLINE constexpr
            void operator()(DataItem &&item) noexcept {
                enter(item);
                _buffer.push_back(std::move(item));
            }

            DANCE_UI_INLINE constexpr
            void operator()(IndirectItem &&item) noexcept {
                enter(item);
                _buffer.push_back(std::move(item));
            }

            DANCE_UI_INLINE constexpr
            void operator()(EqualsItem &&item) noexcept {
                enter(item);
                _buffer.push_back(std::move(item));
            }

            DANCE_UI_INLINE constexpr
            void operator()(HeapRefItem &&item) noexcept {
                enter(item);
                _buffer.push_back(std::move(item));
            }

            DANCE_UI_INLINE constexpr
            void operator()(EnumItem &&item) noexcept {
                enter(item);
                _buffer.push_back(std::move(item));
            }

            DANCE_UI_INLINE constexpr
            void operator()(ExistentialItem &&item) noexcept {
                enter(item);
                _buffer.push_back(std::move(item));
            }

            DANCE_UI_INLINE constexpr
            void operator()(NestedItem &&item) noexcept {
                enter(item);
                _buffer.push_back(std::move(item));
            }

            DANCE_UI_INLINE constexpr
            void enter(const RangeItem &item) noexcept {
                if (_last_range_item.start_offset + _last_range_item.instance_size + _last_range_item.skip_size != item.start_offset) {
                    _buffer.push_back(DataItem(_last_range_item.start_offset + _last_range_item.instance_size + _last_range_item.skip_size,
                                               item.start_offset - _last_range_item.start_offset - _last_range_item.instance_size - _last_range_item.skip_size,
                                               0,
                                               true));
                }
                _last_range_item = item;
            }

            DANCE_UI_INLINE  constexpr
            Container &data() noexcept {
                return _buffer;
            }

            DANCE_UI_INLINE constexpr
            const Container &data() const noexcept {
                return _buffer;
            }

        private:

            Container _buffer;


            Builder::ElementType _currentItem;
            Builder::RangeItem _last_range_item;
        };

    public:
        DANCE_UI_INLINE
        Builder(const DanceUIComparisonMode comparisonMode,
                const HeapMode heapMode) noexcept;

    public:

        bool should_visit_fields(const Swift::metadata *metadata, bool using_equatable);


        virtual bool visit_element(const Swift::metadata *metadata,
                                   TypeReferenceKind kind,
                                   size_t offset,
                                   size_t remaining_size) noexcept;


        virtual bool visit_case(const Swift::metadata *metadata,
                                const Swift::field_record &field_record,
                                size_t caseIndex) noexcept;


        virtual bool visit_existential(const Swift::existential_type_metadata *metadata) noexcept;


        virtual bool visit_function(const Swift::function_type_metadata *metadata) noexcept;


        virtual bool visit_native_object(const Swift::metadata *metadata) noexcept;

        void add_field(size_t size) noexcept;

        LayoutBufferType
        commit(const Swift::metadata *metadata) noexcept;

        template <typename ItemType>
        DANCE_UI_INLINE constexpr
        void push_back(const ItemType &item) noexcept {
            if (_fieldsBuffer) {
                _fieldsBuffer->push_back(item);
                return;
            }
            data().push_back(item);
        }

        template <typename ItemType>
        DANCE_UI_INLINE constexpr
        void push_back(ItemType &&item) noexcept {
            if (_fieldsBuffer) {
                _fieldsBuffer->push_back(std::forward<ItemType>(item));
                return;
            }
            data().push_back(std::forward<ItemType>(item));
        }

        DANCE_UI_INLINE constexpr
        void pop_back() noexcept {
            if (_fieldsBuffer) {
                _fieldsBuffer->pop_back();
                return;
            }
            data().pop_back();
        }

        DANCE_UI_INLINE constexpr
        size_t size() const noexcept {
            if (_fieldsBuffer) {
                return _fieldsBuffer->size();
            }
            return data().size();
        }

        DANCE_UI_INLINE constexpr
        bool empty() const noexcept {
            return size() == 0;
        }

        DANCE_UI_INLINE
        const ElementType &back() const noexcept {
            if (_fieldsBuffer) {
                return _fieldsBuffer->back();
            }
            return data().back();
        }

        DANCE_UI_INLINE
        ElementType &back() noexcept {
            if (_fieldsBuffer) {
                return _fieldsBuffer->back();
            }
            return data().back();
        }

        DANCE_UI_INLINE constexpr
        const DanceUIComparisonMode &get_comparison_mode() const noexcept {
            return _comparisonMode;
        }

        class ComparisonModeRAII {
        public:
            DANCE_UI_INLINE
            ComparisonModeRAII(Builder &builder, const Swift::metadata *metadata) noexcept :
            _builder(builder),
            _comparisonMode(builder.get_comparison_mode()) {
                builder._set_comparison_mode(mode_for_type(metadata, _comparisonMode));
            }

            DANCE_UI_INLINE
            ~ComparisonModeRAII() noexcept {
                _builder._set_comparison_mode(_comparisonMode);
            }

        private:
            Builder &_builder;
            const DanceUIComparisonMode _comparisonMode;
        };

        class OffsetRAII {
        public:
            DANCE_UI_INLINE
            OffsetRAII(Builder &builder, size_t delta) noexcept :
            _builder(builder),
            _offset(_builder.get_offset()) {
                _builder.set_offset(_offset + delta);
            }

            DANCE_UI_INLINE
            ~OffsetRAII() noexcept {
                _builder.set_offset(_offset);
            }

        private:
            Builder &_builder;
            const size_t _offset;
        };

    protected:

        DANCE_UI_INLINE
        LayoutBufferType &data() noexcept {
            return _buffer;
        }

        DANCE_UI_INLINE
        const LayoutBufferType &data() const noexcept {
            return _buffer;
        }

        DANCE_UI_INLINE constexpr
        const HeapMode &_get_heap_mode() const noexcept {
            return _heapMode;
        }

        DANCE_UI_INLINE constexpr
        const size_t &get_offset() const noexcept {
            return _offset;
        }

        DANCE_UI_INLINE constexpr
        void set_offset(size_t new_offset) noexcept {
            _offset = new_offset;
        }

    private:
        friend class ComparisonModeRAII;
        friend class OffsetRAII;

        DANCE_UI_INLINE constexpr
        void _set_comparison_mode(DanceUIComparisonMode mode) noexcept {
            _comparisonMode = mode;
        }


        DanceUIComparisonMode _comparisonMode;


        const HeapMode _heapMode;


        size_t _offset;


        ElementTypeBufferType *_fieldsBuffer;


        LayoutBufferType _buffer;


    };

    using LayoutBufferType = Builder::LayoutBufferType;

public:

    DANCE_UI_INLINE
    const LayoutBufferType &getBuffer() const noexcept {
        return _buffer;
    }

    DANCE_UI_INLINE
    bool operator==(const LayoutDescriptor &rhs) const noexcept {
        return _type == rhs._type &&
        _buffer == rhs._buffer;
    }

    bool compare(const OpaqueValue *lhs,
                 const OpaqueValue *rhs,
                 const DanceUIComparisonMode comparisonMode,
                 const HeapMode heapMode) const noexcept;

    bool compare_partial(const OpaqueValue *lhs,
                         const OpaqueValue *rhs,
                         size_t skipBytes,
                         size_t compareBytes,
                         const DanceUIComparisonMode comparisonMode,
                         const HeapMode heapMode) const noexcept;

    bool compare_indirect(const TypeLayout *valueLayout,
                          const EnumMetadata *enumType,
                          const Metadata *type,
                          const DanceUIComparisonMode comparisonMode,
                          const size_t skipBytes,
                          const size_t compareBytes,
                          const OpaqueValue *lhs,
                          const OpaqueValue *rhs) const noexcept;

    static bool compare_heap_objects(const swift::OpaqueValue *lhs,
                                     const swift::OpaqueValue *rhs,
                                     const DanceUIComparisonMode comparisonMode,
                                     bool is_capture_ref) noexcept;

    static bool compare_existential_values(const swift::Metadata *type,
                                           const OpaqueValue *lhs,
                                           const OpaqueValue *rhs,
                                           const DanceUIComparisonMode comparisonMode) noexcept;

    void dump() const noexcept;

    void dump(std::ostream &os) const noexcept;

    static
    void add_type_descriptor_override(const void *type_descriptor,
                                      DanceUIComparisonMode comparison_mode);

    static LayoutBufferType
    make_layout(const Swift::metadata *metadata,
                DanceUIComparisonMode comparisonMode,
                HeapMode heapMode,
                bool &invalidate) noexcept;

    enum class CacheSlot: size_t {
        Default = 0,
        FromCompareHeapObjectOrShouldVisit = 1,
        FromInternType = std::numeric_limits<size_t>::max(),
    };

    static
    const LayoutDescriptor &fetch(const Swift::metadata *metadata,
                                  DanceUIComparisonMode comparisonMode,
                                  LayoutDescriptor::CacheSlot slot = LayoutDescriptor::CacheSlot::Default) noexcept;

    static const LayoutDescriptor &get_invalid() noexcept;

    DANCE_UI_INLINE
    LayoutDescriptor(const Metadata *type,
                     LayoutBufferType &&buffer) noexcept :
    _type(type),
    _buffer(std::move(buffer)) {
    }

    LayoutDescriptor(const LayoutDescriptor &value) = delete;

    DANCE_UI_INLINE
    LayoutDescriptor(LayoutDescriptor &&value) noexcept {
        *this = std::move(value);
    }

    DANCE_UI_INLINE
    LayoutDescriptor &operator=(LayoutDescriptor &&value) noexcept {
        if (this != &value) {
            _type = value._type;
            _buffer = std::move(value._buffer);
        }
        return *this;
    }

private:

    friend class TypeDescriptorCache;

    const Metadata *_type;
    LayoutBufferType _buffer;
};

}

#endif
