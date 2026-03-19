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

#ifndef MetadataVisitor_hpp
#define MetadataVisitor_hpp

#include <swift/Runtime/Metadata.h>
#include <swift/Runtime/Casting.h>
#include <swift/Runtime/HeapObject.h>
#include <swift/Runtime/ExistentialContainer.h>

#include <DanceUIRuntime/DanceUI_Debug.hpp>
#include <DanceUIRuntime/DanceUIEquatable.hpp>

#include <CoreFoundation/CoreFoundation.h>

using namespace swift;


namespace DanceUI {
namespace Swift {

using Any = OpaqueExistentialContainer;

class metadata_visitor;

using field_record = reflection::FieldRecord;

class metadata : public swift::Metadata {
public:

    DANCE_UI_INLINE
    TypeNamePair name(bool qualified) const noexcept {
        return swift_getTypeName(this, qualified);
    }

    DANCE_UI_INLINE
    MetadataKind getKind() const noexcept {
        return getType()->getKind();
    }

    DANCE_UI_INLINE
    const TypeContextDescriptor *descriptor() const noexcept {
        return getType()->getTypeContextDescriptor();
    }

    DANCE_UI_INLINE
    const TypeContextDescriptor *nominal_descriptor() const noexcept {
        auto description = descriptor();
        if (!description) {
            return nullptr;
        }

        switch (description->getKind()) {
            case swift::ContextDescriptorKind::Class:
                return description;
            case swift::ContextDescriptorKind::Struct:
                return description;
            case swift::ContextDescriptorKind::Enum:
                return description;
            default:
                return nullptr;
        }
    }

    WitnessTablePointer equatable() const noexcept;

    const Metadata *mangled_type_name_ref(const char *typeName,
                                          TypeReferenceKind *ref_kind_ptr) const noexcept;

    DANCE_UI_INLINE
    const Metadata *getType() const noexcept {
        return this;
    }

    void append_description(CFMutableStringRef description) const noexcept;

    void copy_on_write_heap_object(void *&value) const noexcept;

public:
    bool visit(metadata_visitor &visitor) const noexcept;

    bool visit_heap(metadata_visitor &visitor, uint32_t heapMode) const noexcept;
    bool visit_heap_locals(metadata_visitor &visitor) const noexcept;
    bool visit_heap_class(metadata_visitor &visitor) const noexcept;

protected:
    friend class metadata_visitor;
    static WitnessTablePointer nsobject_conformance;
    static const Metadata *native_object;

private:
    metadata() noexcept = delete;
};

class existential_type_metadata final: public metadata {
public:

    ExistentialTypeRepresentation representation() const noexcept {
        const auto type = getType();
        switch (type->Flags.getSpecialProtocol()) {
            case SpecialProtocol::Error: {
                return ExistentialTypeRepresentation::Error;
            }
            case SpecialProtocol::None: {
                break;
            }
        }


        if (type->isClassBounded()) {
            return ExistentialTypeRepresentation::Class;
        }
        return ExistentialTypeRepresentation::Opaque;
    }

    const OpaqueValue *project_value(const OpaqueValue *container) const noexcept;

    const Metadata *dynamic_type(const OpaqueValue *container) const noexcept;

    DANCE_UI_INLINE
    const ExistentialTypeMetadata *getType() const noexcept {
        return cast<ExistentialTypeMetadata>(metadata::getType());
    }

private:
    existential_type_metadata() noexcept = delete;

};

class any_class_type_metadata final: public metadata {
public:

    DANCE_UI_INLINE
    const ClassMetadata *getType() const noexcept {
        return cast<ClassMetadata>(metadata::getType());
    }

private:
    any_class_type_metadata() noexcept = delete;
};

class function_type_metadata final: public metadata {
public:

    DANCE_UI_INLINE
    const FunctionTypeMetadata *getType() const noexcept {
        return cast<FunctionTypeMetadata>(metadata::getType());
    }

private:
    function_type_metadata() noexcept = delete;
};

class metadata_visitor {
public:

    metadata_visitor() noexcept {
    }
    virtual ~metadata_visitor() noexcept {}


    virtual bool unknown_result() const noexcept {
        return false;
    }


    virtual bool visit_element(const metadata *metadata,
                               const TypeReferenceKind ref_kind,
                               size_t offset,
                               size_t remaining_size) noexcept;


    virtual bool visit_field(const metadata *metadata,
                             const field_record &field_record,
                             size_t offset,
                             size_t remaining_size) noexcept;


    virtual bool visit_case(const metadata *metadata,
                            const field_record &field_record,
                            size_t caseIndex) noexcept;


    virtual bool visit_class(const any_class_type_metadata *metadata) noexcept;


    virtual bool visit_existential(const existential_type_metadata *metadata) noexcept;


    virtual bool visit_function(const function_type_metadata *metadata) noexcept;


    virtual bool visit_native_object(const metadata *metadata) noexcept;

};


}
}

#endif
