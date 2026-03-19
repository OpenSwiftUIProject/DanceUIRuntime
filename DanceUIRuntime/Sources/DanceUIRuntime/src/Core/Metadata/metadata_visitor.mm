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
#include "LayoutDescriptor.hpp"

#include <public/runtime/ErrorObject.h>
#include <public/runtime/Private.h>

#include <swift/Basic/Unreachable.h>
#include <swift/Basic/Lazy.h>
#include <swift/Runtime/ObjCBridge.h>

using namespace swift;


SWIFT_ATTRIBUTE_NORETURN
void
swift::fatalError(uint32_t flags, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    char *log;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
    swift_vasprintf(&log, format, args);
#pragma GCC diagnostic pop

    swift_reportError(flags, log);
    abort();
}

#if SWIFT_OBJC_INTEROP
static ClassMetadataBounds computeMetadataBoundsForObjCClass(Class cls) {
    cls = swift_getInitializedObjCClass(cls);
    auto metadata = reinterpret_cast<const ClassMetadata *>(cls);
    return metadata->getClassBoundsAsSwiftSuperclass();
}
#endif

static ClassMetadataBounds
computeMetadataBoundsForSuperclass(const void *ref,
                                   TypeReferenceKind refKind) {
    switch (refKind) {
        case TypeReferenceKind::IndirectTypeDescriptor: {
            auto description = *reinterpret_cast<const ClassDescriptor * const *>(ref);
            if (!description) {
                swift::fatalError(0, "instantiating class metadata for class with "
                                  "missing weak-linked ancestor");
            }
            return description->getMetadataBounds();
        }

        case TypeReferenceKind::DirectTypeDescriptor: {
            auto description = reinterpret_cast<const ClassDescriptor *>(ref);
            return description->getMetadataBounds();
        }

        case TypeReferenceKind::DirectObjCClassName: {
#if SWIFT_OBJC_INTEROP
            auto cls = objc_lookUpClass(reinterpret_cast<const char *>(ref));
            return computeMetadataBoundsForObjCClass(cls);
#else
            break;
#endif
        }

        case TypeReferenceKind::IndirectObjCClass: {
#if SWIFT_OBJC_INTEROP
            auto cls = *reinterpret_cast<const Class *>(ref);
            return computeMetadataBoundsForObjCClass(cls);
#else
            break;
#endif
        }
    }
    swift_unreachable("unsupported superclass reference kind");
}

static ClassMetadataBounds computeMetadataBoundsFromSuperclass(
                                                               const ClassDescriptor *description,
                                                               StoredClassMetadataBounds &storedBounds) {
    ClassMetadataBounds bounds;


    if (const void *superRef = description->getResilientSuperclass()) {
        bounds = computeMetadataBoundsForSuperclass(superRef,
                                                    description->getResilientSuperclassReferenceKind());
    } else {
        bounds = ClassMetadataBounds::forSwiftRootClass();
    }


    bounds.adjustForSubclass(description->areImmediateMembersNegative(),
                             description->NumImmediateMembers);


    storedBounds.initialize(bounds);
    return bounds;
}

ClassMetadataBounds
swift::getResilientMetadataBounds(const ClassDescriptor *description) {
    assert(description->hasResilientSuperclass());
    auto &storedBounds = *description->ResilientMetadataBounds.get();

    ClassMetadataBounds bounds;
    if (storedBounds.tryGet(bounds)) {
        return bounds;
    }

    return computeMetadataBoundsFromSuperclass(description, storedBounds);
}

int32_t
swift::getResilientImmediateMembersOffset(const ClassDescriptor *description) {
    assert(description->hasResilientSuperclass());
    auto &storedBounds = *description->ResilientMetadataBounds.get();

    ptrdiff_t result;
    if (storedBounds.tryGetImmediateMembersOffset(result)) {
        return int32_t(result / sizeof(void*));
    }

    auto bounds = computeMetadataBoundsFromSuperclass(description, storedBounds);
    return int32_t(bounds.ImmediateMembersOffset / sizeof(void*));
}

static Class swift_getNSErrorClass() {
    return SWIFT_LAZY_CONSTANT(objc_lookUpClass("NSError"));
}

static const ClassMetadata *swift_getClass(const void *object) {
#if SWIFT_OBJC_INTEROP
    if (!isObjCTaggedPointer(object))
        return _swift_getClassOfAllocated(object);
    return reinterpret_cast<const ClassMetadata*>(
                                                  object_getClass(id_const_cast(object)));
#else
    return _swift_getClassOfAllocated(object);
#endif
}

extern "C" const ProtocolDescriptor PROTOCOL_DESCR_SYM(SQ);
static constexpr auto &EquatableProtocolDescriptor = PROTOCOL_DESCR_SYM(SQ);

SWIFT_CC(swift) SWIFT_RUNTIME_EXPORT
const Metadata *
swift_getTypeByMangledNameInContext(
                                    const char *typeNameStart,
                                    size_t typeNameLength,
                                    const TargetContextDescriptor<InProcess> *context,
                                    const void * const *genericArgs);

namespace DanceUI {
namespace Swift {

WitnessTablePointer metadata::nsobject_conformance = nullptr;
const Metadata *metadata::native_object = nullptr;

WitnessTablePointer metadata::equatable() const noexcept {


    const auto kind = getType()->getKind();
    if (kind >= MetadataKind::Existential) {
        do {
            if (kind == MetadataKind::Tuple) {
                break;
            }
            if (!nsobject_conformance) {
                const ClassMetadata *cls =
                reinterpret_cast<const ClassMetadata *>(objc_getClass("NSObject"));
                if (const Metadata *objcMetadata = swift_getObjCClassMetadata((cls))) {
                    nsobject_conformance = swift_conformsToProtocol(objcMetadata,
                                                                    &EquatableProtocolDescriptor);
                }
            }
            const WitnessTablePointer result = swift_conformsToProtocol(getType(),
                                                                        &EquatableProtocolDescriptor);
            if (nsobject_conformance != result) {
                return nullptr;
            }
            return result;
        } while (0);
    }
    return swift_conformsToProtocol(getType(), &EquatableProtocolDescriptor);
}

DANCE_UI_INLINE
static llvm::StringRef makeSymbolicMangledNameStringRef(const char *base) noexcept {
    if (!base) {
        return {};
    }
    auto end = base;
    while (*end != '\0') {

        if (*end >= '\x01' && *end <= '\x17')
            end += sizeof(uint32_t);
        else if (*end >= '\x18' && *end <= '\x1F')
            end += sizeof(void*);
        ++end;
    }
    return StringRef(base, end - base);
}

static Class getAndBridgeSwiftNativeNSErrorClass() noexcept {
    Class nsErrorClass = swift_getNSErrorClass();
    Class ourClass = objc_getClass("__SwiftNativeNSError");

    if (nsErrorClass) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        class_setSuperclass(ourClass, nsErrorClass);
#pragma clang diagnostic pop
    }
    return ourClass;
}

static Class getSwiftNativeNSErrorClass() noexcept {
    return SWIFT_LAZY_CONSTANT(getAndBridgeSwiftNativeNSErrorClass());
}

DANCE_UI_INLINE
bool SwiftError_isPureNSError(const SwiftError *error) noexcept {
    return
#if SWIFT_OBJC_INTEROP


    swift_getClass(error) != (ClassMetadata *)getSwiftNativeNSErrorClass();
#else
    errorBox->isPureNSError();
#endif
}


DANCE_UI_INLINE
const OpaqueValue *SwiftError_getValue(const SwiftError *error) noexcept {


    assert(!SwiftError_isPureNSError(error));

    auto baseAddr = reinterpret_cast<uintptr_t>(error + 1);

    unsigned alignMask = unsigned(error->type->getValueWitnesses()->getAlignmentMask());
    baseAddr = (baseAddr + alignMask) & ~(uintptr_t)alignMask;
    return reinterpret_cast<const OpaqueValue *>(baseAddr);
}

DANCE_UI_INLINE
OpaqueValue *SwiftError_getValue(SwiftError *error) noexcept {
    return const_cast<OpaqueValue*>(SwiftError_getValue(
                                                        const_cast<const SwiftError *>(error)));
}

static
const OpaqueValue *OpaqueExistentialContainer_projectValue(const OpaqueExistentialContainer *container) noexcept {
    auto *vwt = container->Type->getValueWitnesses();

    if (vwt->isValueInline())
        return reinterpret_cast<const OpaqueValue *>(&container->Buffer);


    unsigned alignMask = unsigned(vwt->getAlignmentMask());
    unsigned byteOffset = (sizeof(HeapObject) + alignMask) & ~alignMask;
    auto *bytePtr = reinterpret_cast<const char *>(
                                                   *reinterpret_cast<HeapObject *const *const>(&container->Buffer));
    return reinterpret_cast<const OpaqueValue *>(bytePtr + byteOffset);
}

const OpaqueValue *
existential_type_metadata::project_value(const OpaqueValue *container) const noexcept {
    switch (representation()) {
        case ExistentialTypeRepresentation::Class: {
            auto classContainer =
            reinterpret_cast<const ClassExistentialContainer *>(container);
            return reinterpret_cast<const OpaqueValue *>(&classContainer->Value);
        }
        case ExistentialTypeRepresentation::Opaque: {
            auto *opaqueContainer =
            reinterpret_cast<const OpaqueExistentialContainer *>(container);
            return OpaqueExistentialContainer_projectValue(opaqueContainer);
        }
        case ExistentialTypeRepresentation::Error: {
            return nullptr;
        }
    }

    swift_unreachable("Unhandled ExistentialTypeRepresentation in switch.");
}

const Metadata *
existential_type_metadata::dynamic_type(const OpaqueValue *container) const noexcept {


    switch (representation()) {
        case ExistentialTypeRepresentation::Class: {
            auto classContainer =
            reinterpret_cast<const ClassExistentialContainer*>(container);
            void *obj = classContainer->Value;
            return swift_getObjectType(reinterpret_cast<HeapObject*>(obj));
        }
        case ExistentialTypeRepresentation::Opaque: {
            auto opaqueContainer =
            reinterpret_cast<const OpaqueExistentialContainer*>(container);
            return opaqueContainer->Type;
        }
        case ExistentialTypeRepresentation::Error: {
            return nullptr;
        }
    }

    swift_unreachable("Unhandled ExistentialTypeRepresentation in switch.");
}

const Metadata *
metadata::mangled_type_name_ref(const char *typeName,
                                TypeReferenceKind *ref_kind_ptr) const noexcept {
    if (typeName == nullptr) {
        return nullptr;
    }
    const auto &description = this->descriptor();
    const void *const *genericArgs = nullptr;
    if (description && description->isGeneric()) {
        const auto kind = description->getKind();
        if (kind <= ContextDescriptorKind::Enum) {
            if (kind == ContextDescriptorKind::Class) {

            } else {
                genericArgs = reinterpret_cast<const void *const *>
                (description->getGenericArguments(getType()));
            }
        }
    }


    const auto nameString = makeSymbolicMangledNameStringRef(typeName);
    const size_t typeNameLength = nameString.size();

    const Metadata *metadata =
    swift_getTypeByMangledNameInContext(typeName,
                                        typeNameLength,
                                        description,
                                        genericArgs);
    if (ref_kind_ptr != nullptr && metadata != nullptr) {
        *ref_kind_ptr = TypeReferenceKind::DirectTypeDescriptor;
        if (typeNameLength >= 3 &&
            (*(typeName + typeNameLength - 2) == 'X' )) {
            const char sign = *(typeName + typeNameLength - 1);
            if (sign == 'w' ) {
                *ref_kind_ptr = TypeReferenceKind::IndirectTypeDescriptor;
            } else if (sign == 'u' ) {
                *ref_kind_ptr = TypeReferenceKind::IndirectObjCClass;
            } else if (sign == 'o' ) {
                *ref_kind_ptr = TypeReferenceKind::DirectObjCClassName;
            }
        }
    }
    return metadata;
}

static_assert(offsetof(const TypeContextDescriptor, Name) == 0x08);
static_assert(offsetof(const TypeContextDescriptor, AccessFunctionPtr) == 0x0c);
static_assert(offsetof(const TypeContextDescriptor, Fields) == 0x10);

static_assert(sizeof(ValueTypeDescriptor) == 0x14);

static_assert(offsetof(const StructDescriptor, NumFields) == 0x14);
static_assert(offsetof(const StructDescriptor, FieldOffsetVectorOffset) == 0x18);
static_assert(sizeof(StructDescriptor) == 0x1c);

static_assert(offsetof(const EnumDescriptor, NumPayloadCasesAndPayloadSizeOffset) == 0x14);
static_assert(offsetof(const EnumDescriptor, NumEmptyCases) == 0x18);
static_assert(sizeof(EnumDescriptor) == 0x1c);

static_assert(offsetof(const TypeMetadataHeader, layoutString) == 0x00);
static_assert(offsetof(const TypeMetadataHeader, ValueWitnesses) == 0x08);

static_assert(offsetof(const ContextDescriptor, Flags) == 0);
static_assert(offsetof(const ContextDescriptor, Parent) == 0x04);

void metadata::append_description(CFMutableStringRef description) const noexcept {
    const auto kind = getKind();
    switch (kind) {
        case MetadataKind::Function: {
            auto functionType = static_cast<const FunctionTypeMetadata *>(getType());
            CFStringAppendCString(description, "(", kCFStringEncodingUTF8);
            const auto parameterCount = functionType->getNumParameters();
            const auto params = functionType->getParameters();
            for (size_t idx = 0; idx < parameterCount; idx++) {
                if (idx != 0) {
                    CFStringAppendCString(description, ", ", kCFStringEncodingUTF8);
                }
                const auto paramType = params[idx];
                if (paramType) {
                    reinterpret_cast<const Swift::metadata *>(paramType)->append_description(description);
                } else {
                    CFStringAppendCString(description, "nil", kCFStringEncodingUTF8);
                }
            }
            CFStringAppendCString(description, ")", kCFStringEncodingUTF8);
            CFStringAppendCString(description, " -> ", kCFStringEncodingUTF8);
            if (functionType->ResultType == nullptr) {
                CFStringAppendCString(description, "nil", kCFStringEncodingUTF8);
                return;
            }
            return reinterpret_cast<const Swift::metadata *>(functionType->ResultType)->append_description(description);
        }
        case MetadataKind::Tuple: {
            auto tupleType = static_cast<const TupleTypeMetadata *>(getType());
            const size_t elementsCount = tupleType->NumElements;
            CFStringAppendCString(description, "(", kCFStringEncodingUTF8);
            const auto elements = tupleType->getElements();
            for (size_t idx = 0; idx < elementsCount; idx++) {
                if (idx != 0) {
                    CFStringAppendCString(description, ", ", kCFStringEncodingUTF8);
                }
                const auto elementType = elements[idx].Type;
                if (elementType) {
                    reinterpret_cast<const Swift::metadata *>(elementType)->append_description(description);
                } else {
                    CFStringAppendCString(description, "nil", kCFStringEncodingUTF8);
                }
            }
            return CFStringAppendCString(description, ")", kCFStringEncodingUTF8);
        }
        default: {
            const TypeContextDescriptor *typeDescriptor = this->descriptor();
            vector<TypeNamePair, 5> namesBuffer;
            CFMutableStringRef genericParamsDescription = NULL;
            if (!typeDescriptor) {
                namesBuffer.push_back(name(false));
            } else {
                const ContextDescriptor *descriptor = typeDescriptor;
                do {
                    const ContextDescriptorKind descriptorKind = descriptor->getKind();
                    switch (descriptorKind) {
                        case swift::ContextDescriptorKind::Class:
                        case swift::ContextDescriptorKind::Enum:
                        case swift::ContextDescriptorKind::Struct: {
                            const TypeContextDescriptor *typeDescriptor = static_cast<const TypeContextDescriptor *>(descriptor);
                            if (!typeDescriptor->Name.isNull()) {
                                namesBuffer.push_back({
                                    typeDescriptor->Name.get(),
                                    0
                                });
                            }
                            break;
                        }
                        default:
                            break;
                    }
                    descriptor = descriptor->Parent.get();
                } while (descriptor != nullptr);
                if (namesBuffer.empty()) {
                    namesBuffer.push_back(name(false));
                }

                const uint32_t genericParamsSize = typeDescriptor->getNumGenericParams();
                if (genericParamsSize != 0) {
                    if (auto genericArgs = getType()->getGenericArgs()) {
                        genericParamsDescription = CFStringCreateMutable(kCFAllocatorDefault, 0);
                        bool shouldAppendSeparator = false;
                        CFStringAppendCString(genericParamsDescription,
                                              "<",
                                              kCFStringEncodingUTF8);
                        for (uint32_t idx = 0; idx < genericParamsSize; idx++) {
                            if (!shouldAppendSeparator) {
                                shouldAppendSeparator = true;
                            } else {
                                CFStringAppendCString(genericParamsDescription,
                                                      ", ",
                                                      kCFStringEncodingUTF8);
                            }
                            reinterpret_cast<const Swift::metadata *>(genericArgs[idx])->append_description(genericParamsDescription);
                        }
                        CFStringAppendCString(genericParamsDescription, ">",
                                              kCFStringEncodingUTF8);
                    }
                }
            }

            const uint32_t namesBufferSize = namesBuffer.size();
            if (namesBufferSize > 1) {
                bool shouldAppendSeparator = false;
                for (uint32_t idx = 0; idx < namesBufferSize; idx++) {
                    if (!shouldAppendSeparator) {
                        shouldAppendSeparator = true;
                    } else {
                        CFStringAppendCString(description, ".",
                                              kCFStringEncodingUTF8);
                    }
                    CFStringAppendCString(description,
                                          namesBuffer[idx].data,
                                          kCFStringEncodingUTF8);
                }
            } else if (namesBufferSize == 1) {
                CFStringAppendCString(description,
                                      namesBuffer[0].data,
                                      kCFStringEncodingUTF8);
            }

            if (genericParamsDescription) {
                CFStringAppend(description, genericParamsDescription);
                CFRelease(genericParamsDescription);
                genericParamsDescription = NULL;
            }
            break;
        }
    }
}

void metadata::copy_on_write_heap_object(void *&value) const noexcept {
    if (swift_isUniquelyReferencedNonObjC(value)) {
        return;
    }
    HeapObject *heap_object = reinterpret_cast<HeapObject *>(value);

    const uintptr_t alignment_mask = getType()->getValueWitnesses()->getAlignmentMask();
    const uintptr_t offset = ((alignment_mask + 0x10) & (~alignment_mask));
    const size_t required_size = getType()->vw_size() + offset;
    HeapObject *new_object = swift_allocObject(heap_object->metadata,
                                               required_size,
                                               alignment_mask | 0x07);

    getType()->vw_initializeWithCopy(reinterpret_cast<OpaqueValue *>(reinterpret_cast<uint8_t *>(new_object) + offset),
                                     reinterpret_cast<OpaqueValue *>(reinterpret_cast<uint8_t *>(heap_object) + offset));

    swift_release(heap_object);
    value = new_object;
}


bool metadata::visit(metadata_visitor &visitor) const noexcept {
    __DANCE_UI_RUNTIME_TYPEDESCRIPTOR_ACCESS_LOG__
    const auto type = getType();

    const MetadataKind typeKind = type->getKind();

    switch (typeKind) {
        case MetadataKind::Class: {
            return visitor.visit_class(reinterpret_cast<const any_class_type_metadata *>(type));
        }
        case MetadataKind::Struct: {

            const auto description = static_cast<const StructDescriptor *>(descriptor());
            if (description == nullptr) {
                return visitor.unknown_result();
            }
            auto structType = static_cast<const StructMetadata *>(type);
            auto fieldsOffsets = structType->getFieldOffsets();

            const ContextDescriptorKind descriptionKind = description->getKind();

            if (descriptionKind > ContextDescriptorKind::Enum ||
                !description->isReflectable()) {
                return visitor.unknown_result();
            }
            const auto fieldsPtr = description->Fields.get();
            if (!fieldsPtr) {
                return visitor.unknown_result();
            }
            const auto fieldsCount = description->NumFields;
            const auto &fields = fieldsPtr->getFields();
            const size_t value_size = vw_size();
            for (size_t idx = 0; idx < fieldsCount; idx++) {
                if (!visitor.visit_field(this,
                                         fields[idx],
                                         fieldsOffsets[idx],
                                         value_size - fieldsOffsets[idx])) {
                    return visitor.unknown_result();
                }
            }
            return true;
        }
        case MetadataKind::Opaque: {
            if (native_object == nullptr) {
                native_object = mangled_type_name_ref("Bo", nullptr);
            }
            if (native_object == type) {

                return visitor.visit_native_object(reinterpret_cast<const Swift::metadata *>(type));
            }
            return visitor.unknown_result();
        }
        case MetadataKind::Tuple: {
            auto tupleType = static_cast<const TupleTypeMetadata *>(type);
            const auto count = tupleType->NumElements;
            const auto elements = tupleType->getElements();
            for (size_t idx = 0; idx < count; idx++) {
                auto elementType = elements[idx].Type;
                if (!elementType) {
                    continue;
                }
                const size_t value_size = elementType->vw_size();
                if (!visitor.visit_element(reinterpret_cast<const Swift::metadata *>(elementType),
                                           TypeReferenceKind::DirectTypeDescriptor,
                                           elements[idx].Offset,
                                           value_size)) {
                    return false;
                }
            }
            return true;
        }
        case MetadataKind::Function: {
            return visitor.visit_function(reinterpret_cast<const Swift::function_type_metadata *>(type));
        }
        case MetadataKind::Existential: {
            return visitor.visit_existential(reinterpret_cast<const Swift::existential_type_metadata *>(type));
        }
        case MetadataKind::Optional:
        case MetadataKind::Enum: {
            const auto description = static_cast<const EnumDescriptor *>
            (descriptor());
            if (!description) {
                return visitor.unknown_result();
            }

            const ContextDescriptorKind descriptionKind =
            description->getKind();

            if (descriptionKind > ContextDescriptorKind::Enum ||
                !description->isReflectable()) {
                return visitor.unknown_result();
            }
            const auto fieldsPtr = description->Fields.get();
            if (!fieldsPtr) {
                return visitor.unknown_result();
            }

            if (0 == description->getNumPayloadCases()) {
                return visitor.unknown_result();
            }

            const auto fieldsCount = fieldsPtr->NumFields;
            const auto &fields = fieldsPtr->getFields();


            for (size_t idx = 0; idx < fieldsCount; idx++) {
                if (!fields[idx].hasMangledTypeName()) {

                    continue;
                }
                if (!visitor.visit_case(this,
                                        fields[idx],
                                        idx)) {


                    return visitor.unknown_result();
                }
            }


            return true;
        }
        case MetadataKind::FixedArray: {
            const FixedArrayTypeMetadata *fixedArrayType = static_cast<const FixedArrayTypeMetadata *>(type);

            const auto count = fixedArrayType->getRealizedCount();
            const auto elementType = fixedArrayType->Element;
            const auto elementSize = elementType->vw_size();
            for (size_t idx = 0; idx < count; idx ++) {
                if (!visitor.visit_element(reinterpret_cast<const Swift::metadata *>(elementType),
                                           TypeReferenceKind::DirectTypeDescriptor,
                                           idx * elementSize,
                                           elementSize)) {
                    return false;
                }
            }
            return true;
        }
        case MetadataKind::ForeignClass:
        case MetadataKind::ForeignReferenceType:
        case MetadataKind::Metatype:
        case MetadataKind::ObjCClassWrapper:
        case MetadataKind::ExistentialMetatype:
        case MetadataKind::ExtendedExistential:
        case MetadataKind::HeapLocalVariable:
        case MetadataKind::HeapGenericLocalVariable:
        case MetadataKind::ErrorObject:
        case MetadataKind::Task:
        case MetadataKind::Job:
        case MetadataKind::LastEnumerated:
            return visitor.unknown_result();
    }
}

bool metadata::visit_heap(metadata_visitor &visitor, uint32_t heapType) const noexcept {
    __DANCE_UI_RUNTIME_TYPEDESCRIPTOR_ACCESS_LOG__
    const MetadataKind kind = getKind();
    switch (kind) {
        case MetadataKind::HeapGenericLocalVariable: {

            if ((heapType & uint32_t(LayoutDescriptor::HeapType::GenericLocalVariable)) == 0) {
                return visitor.unknown_result();
            }
            auto type = static_cast<const GenericBoxHeapMetadata *>(getType());
            auto boxedType = type->BoxedType;
            if (!boxedType) {
                return visitor.unknown_result();
            }

            const auto offset = GenericBoxHeapMetadata::getHeaderOffset(boxedType);
            const size_t value_size = boxedType->vw_size();
            return visitor.visit_element(reinterpret_cast<const Swift::metadata *>(boxedType),
                                         TypeReferenceKind::DirectTypeDescriptor,
                                         offset,
                                         value_size);
        }
        case MetadataKind::HeapLocalVariable: {
            if ((heapType & uint32_t(LayoutDescriptor::HeapType::LocalVariable)) == 0) {
                return visitor.unknown_result();
            }
            return visit_heap_locals(visitor);
        }
        case MetadataKind::Class: {
            if ((heapType & uint32_t(LayoutDescriptor::HeapType::Class)) == 0) {
                return visitor.unknown_result();
            }
            return visit_heap_class(visitor);
        }
        default:
            return visitor.unknown_result();
    }
}

bool metadata::visit_heap_locals(metadata_visitor &visitor) const noexcept {
    __DANCE_UI_RUNTIME_TYPEDESCRIPTOR_ACCESS_LOG__
    auto type = static_cast<const HeapLocalVariableMetadata *>(getType());


    if (type->CaptureDescription == nullptr) {
        return visitor.unknown_result();
    }


    auto captureDescriptor = reinterpret_cast<const reflection::CaptureDescriptor *>(type->CaptureDescription);


    if (captureDescriptor->NumMetadataSources != 0) {
        return visitor.unknown_result();
    }

    static_assert(offsetof(reflection::CaptureDescriptor, NumMetadataSources) == 0x04);
    static_assert(offsetof(HeapLocalVariableMetadata, OffsetToFirstCapture) == 0x08);
    static_assert(offsetof(HeapLocalVariableMetadata, CaptureDescription) == 0x10);


    size_t offsetToCapture = type->OffsetToFirstCapture;

    if (offsetToCapture == 0) {
        return visitor.unknown_result();
    }

    if (captureDescriptor->NumBindings != 0) {
        static const Metadata *mangeld_type = mangled_type_name_ref("Bp",
                                                                    nullptr);
        if (!mangeld_type) {
            return visitor.unknown_result();
        }

        const size_t value_size = mangeld_type->vw_size();
        for (size_t idx = 0; idx < captureDescriptor->NumBindings; idx ++) {
            if (!visitor.visit_element(reinterpret_cast<const Swift::metadata *>(mangeld_type),
                                       TypeReferenceKind::IndirectObjCClass,
                                       offsetToCapture,
                                       value_size)) {
                return false;
            }
            offsetToCapture += sizeof(const TypeContextDescriptor *);
        }
    }
    for (auto it = captureDescriptor->capture_begin();
         it != captureDescriptor->capture_end();
         ++it) {
        TypeReferenceKind ref_kind = TypeReferenceKind::DirectTypeDescriptor;
        auto elementType = static_cast<const Metadata *>(mangled_type_name_ref(it->MangledTypeName, &ref_kind));
        if (elementType == nullptr) {
            return visitor.unknown_result();
        }

        const size_t offset = offsetToCapture;
        const size_t value_size = elementType->vw_size();

        if (!visitor.visit_element(reinterpret_cast<const Swift::metadata *>(elementType),
                                   ref_kind,
                                   offset,
                                   value_size)) {
            return false;
        }


        offsetToCapture = ((offset + value_size - 1) & ~0x7) + 0x8;
    }
    return true;
}

bool metadata::visit_heap_class(metadata_visitor &visitor) const noexcept {
    __DANCE_UI_RUNTIME_TYPEDESCRIPTOR_ACCESS_LOG__
    auto type = static_cast<const ClassMetadata *>(getType());
    if (!type->isPureObjC()) {
        return visitor.unknown_result();
    }


    const ClassDescriptor *description = type->getDescription();
    if (!description) {
        return visitor.unknown_result();
    }


    if (description->SuperclassType != nullptr && type->Superclass != nullptr) {
        if (type->Superclass->getKind() != MetadataKind::Class) {
            return visitor.unknown_result();
        }
        if (!reinterpret_cast<const Swift::metadata *>(type->Superclass)->visit_heap_class(visitor)) {
            return false;
        }
    }
    if (!description->isReflectable()) {
        return true;
    }
    const reflection::FieldDescriptor *fields = description->Fields.get();
    if (!fields) {
        return true;
    }


    if (fields->NumFields != description->NumFields) {
        return visitor.unknown_result();
    }
    if (type->getFlags() & ClassFlags::UsesSwiftRefcounting) {
        unsigned int count = 0;
        Class cls = reinterpret_cast<Class>(const_cast<ClassMetadata *>(type));
        Ivar *ivarPtr = class_copyIvarList(cls, &count);
        if (ivarPtr == nullptr) {
            return visitor.unknown_result();
        }
        ptrdiff_t *offsets = nullptr;
        if (count == fields->NumFields) {
            offsets = (ptrdiff_t *)alloca(sizeof(ptrdiff_t) * count);
            memset(offsets, 0, sizeof(ptrdiff_t) * count);
            for (unsigned int idx = 0; idx < count; idx ++) {
                offsets[idx] = ivar_getOffset(ivarPtr[idx]);
            }
        }
        if (ivarPtr) {
            free(ivarPtr);
        }
        if (offsets == nullptr ||
            offsets[0] == 0) {
            return visitor.unknown_result();
        }
        if (fields->NumFields == 0) {
            return true;
        }
        size_t idx = 0;
        const size_t value_size = vw_size();
        for (auto it = fields->begin(); it != fields->end(); ++it, idx++) {
            if (!visitor.visit_field(this, *it, offsets[idx], value_size - offsets[idx])) {
                return false;
            }
        }
    }
    return true;
}

#pragma mark - Visitor


bool metadata_visitor::visit_element(const metadata *metadata,
                                     const TypeReferenceKind ref_kind,
                                     size_t offset,
                                     size_t remaining_size) noexcept {
    return unknown_result();
}


bool metadata_visitor::visit_field(const metadata *metadata,
                                   const field_record &field_record,
                                   size_t offset,
                                   size_t remaining_size) noexcept {
    if (!field_record.MangledTypeName) {
        return unknown_result();
    }
    TypeReferenceKind kind = TypeReferenceKind::DirectTypeDescriptor;
    const Metadata *field_type = metadata->mangled_type_name_ref(field_record.MangledTypeName.get(),
                                                                 &kind);

    if (field_type == nullptr) {
        return unknown_result();
    }
    const size_t instance_size = field_type->vw_size();
    remaining_size = std::min(remaining_size, instance_size);
    if (field_type->getKind() != swift::MetadataKind::Metatype) {
        remaining_size = instance_size;
    }

    return visit_element(reinterpret_cast<const Swift::metadata *>(field_type), kind, offset, remaining_size);
}


bool metadata_visitor::visit_case(const metadata *metadata,
                                  const field_record &field_record,
                                  size_t caseIndex) noexcept {
    return unknown_result();
}


bool metadata_visitor::visit_class(const any_class_type_metadata *metadata) noexcept {
    return unknown_result();
}


bool metadata_visitor::visit_existential(const existential_type_metadata *metadata) noexcept {
    return unknown_result();
}


bool metadata_visitor::visit_function(const function_type_metadata *metadata) noexcept {
    return unknown_result();
}


bool metadata_visitor::visit_native_object(const metadata *metadata) noexcept {
    return unknown_result();
}

}
}

#include <CoreFoundation/CoreFoundation.h>

extern "C"
CFStringRef DanceUITypeCopyDescription(const Metadata *type) noexcept {
    CFMutableStringRef description = CFStringCreateMutable(nullptr, 0);
    reinterpret_cast<const DanceUI::Swift::metadata *>(type)->append_description(description);
    return description;
}
