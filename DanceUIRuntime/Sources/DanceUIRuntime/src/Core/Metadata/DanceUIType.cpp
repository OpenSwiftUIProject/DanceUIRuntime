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

#include "metadata_visitor.hpp"

#include <boost/optional.hpp>

#pragma mark - Prototypes

CF_EXTERN_C_BEGIN

const void * _Nullable DanceUITypeGetMemberTypeWithOffsetAndTypeSize(const void * _Nonnull metadata, intptr_t offset, size_t typeSize);

CF_EXTERN_C_END

#pragma mark - Implementations

namespace DanceUI {

namespace Swift {

class ValueTypeMemberTypeVisitor: public Swift::metadata_visitor {

    const uintptr_t expected_offset;

    const uintptr_t expected_size;

public:

    uintptr_t walked_offset;

    const Swift::metadata *result_metadata;

    DANCE_UI_INLINE
    ValueTypeMemberTypeVisitor(uintptr_t expected_offset, size_t expected_size):
        expected_offset(expected_offset),
        expected_size(expected_size),
        walked_offset(0),
        result_metadata(nullptr)
    {}

    ~ValueTypeMemberTypeVisitor() { }

private:

    DANCE_UI_INLINE
    bool visit_general(const Swift::metadata *member_metadata, size_t aligned_offset) noexcept {


        if (aligned_offset) {
            walked_offset = aligned_offset;
        }

        const auto *member_type = member_metadata->getType();
        auto member_type_size = member_type->vw_size();

        if (walked_offset == expected_offset && member_type_size == expected_size) {
            result_metadata = member_metadata;
            return false;
        }

        if (walked_offset + member_type_size > expected_offset) {
            ValueTypeMemberTypeVisitor visitor(expected_offset - walked_offset, expected_size);
            member_metadata->visit(visitor);
            if (visitor.result_metadata != nullptr) {
                result_metadata = visitor.result_metadata;
                return false;
            } else {
                return true;
            }
        } else {
            walked_offset += member_type_size;
            return true;
        }
    }

public:

    DANCE_UI_INLINE
    bool unknown_result() const noexcept override {
        return false;
    }

    DANCE_UI_INLINE
    bool visit_element(const metadata *metadata,
                       const TypeReferenceKind ref_kind,
                       size_t offset,
                       size_t remaining_size) noexcept override {
        return visit_general(metadata, offset);
    }

    DANCE_UI_INLINE
    bool visit_field(const Swift::metadata *metadata,
                     const field_record &field_record,
                     size_t offset,
                     size_t remaining_size) noexcept override {
        auto field_metadata = metadata->mangled_type_name_ref(field_record.MangledTypeName.get(), nullptr);
        return visit_general(reinterpret_cast<const class metadata *>(field_metadata), offset);
    }

    DANCE_UI_INLINE
    bool visit_case(const Swift::metadata *metadata,
                    const field_record &field_record,
                    size_t caseIndex) noexcept override {

        result_metadata = nullptr;
        return unknown_result();
    }

    DANCE_UI_INLINE
    bool visit_class(const Swift::any_class_type_metadata *metadata) noexcept override {

        result_metadata = nullptr;
        return unknown_result();
    }

    DANCE_UI_INLINE
    bool visit_existential(const Swift::existential_type_metadata *metadata) noexcept override {

        result_metadata = nullptr;
        return unknown_result();
    }

    DANCE_UI_INLINE
    bool visit_function(const Swift::function_type_metadata *metadata) noexcept override {

        result_metadata = nullptr;
        return unknown_result();
    }

    DANCE_UI_INLINE
    bool visit_native_object(const Swift::metadata *metadata) noexcept override {

        result_metadata = nullptr;
        return unknown_result();
    }

};

}

}

const void * _Nullable DanceUITypeGetMemberTypeWithOffsetAndTypeSize(const void * _Nonnull metadata, intptr_t offset, size_t typeSize) {

    if (typeSize == 0) {
        return nullptr;
    }

    const DanceUI::Swift::metadata *swift_metadata = reinterpret_cast<const DanceUI::Swift::metadata *>(metadata);

    switch (swift_metadata->getKind()) {
        case swift::MetadataKind::Struct:
            LLVM_FALLTHROUGH;
        case swift::MetadataKind::Tuple:
            break;
        default:
            return nullptr;
    }

    if (offset == 0 && swift_metadata->getType()->vw_size() == typeSize) {
        return metadata;
    }

    DanceUI::Swift::ValueTypeMemberTypeVisitor visitor(offset, typeSize);

    swift_metadata->visit(visitor);

    return visitor.result_metadata;
}
