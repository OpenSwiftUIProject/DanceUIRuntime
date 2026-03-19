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

#include <swift/ABI/Metadata.h>

#include "DanceUI_Debug.hpp"
#include "metadata_visitor.hpp"

using namespace swift;
using namespace DanceUI;

extern "C"
size_t DanceUITypeGetSize(const Metadata *type) {
    return type->getValueWitnesses()->getSize();
}

extern "C"
size_t DanceUITypeGetAlignmentMask(const Metadata *type) {
    return type->getValueWitnesses()->getAlignmentMask();
}

extern "C"
size_t DanceUITypeGetStride(const Metadata *type) {
    return type->getValueWitnesses()->getStride();
}

extern "C"
BOOL DanceUITypeIsBitwiseTakable(const Metadata *type) {
    return type->getValueWitnesses()->isBitwiseTakable();
}

extern "C"
void DanceUITypeValueWitnessTableInitialize(OpaqueValue *dst,
                                            OpaqueValue *src,
                                            const Metadata *type) {
    type->getValueWitnesses()->initializeWithCopy(dst, src, type);
}

extern "C"
void DanceUITypeValueWitnessTableMoveInitialize(OpaqueValue *dst,
                                                OpaqueValue *src,
                                                const Metadata *type) {
    type->getValueWitnesses()->initializeWithTake(dst, src, type);
}

extern "C"
void DanceUITypeValueWitnessTableAssign(OpaqueValue *dst,
                                        OpaqueValue *src,
                                        const Metadata *type) {
    type->getValueWitnesses()->assignWithCopy(dst, src, type);
}

extern "C"
void DanceUITypeValueWitnessTableMoveAssign(OpaqueValue *dst,
                                            OpaqueValue *src,
                                            const Metadata *type) {
    type->getValueWitnesses()->assignWithTake(dst, src, type);
}

extern "C"
void DanceUITypeValueWitnessTableDestroy(OpaqueValue *value,
                                         const Metadata *type) {
    type->getValueWitnesses()->destroy(value, type);
}

extern "C"
bool DanceUITypeApplyEnumData(const Metadata *type,
                              OpaqueValue *value,
                              void (*body)(void * SWIFT_CONTEXT,
                                           int64_t enum_tag,
                                           const Metadata *type,
                                           OpaqueValue *value) SWIFT_CC(swift),
                              void *bodyContext) {
    const swift::ValueWitnessTable *vwt = type->getValueWitnesses();
    if (!vwt->flags.hasEnumWitnesses()) {
        precondition_failure("not an enum type: %s", reinterpret_cast<const Swift::metadata *>(type)->name(false).data);
    }

    const swift::EnumValueWitnessTable *enum_vwt = (const swift::EnumValueWitnessTable *)vwt;
    const unsigned enum_tag = enum_vwt->getEnumTag(value, type);

    const swift::EnumDescriptor *nominal_descriptor = (const swift::EnumDescriptor *)(reinterpret_cast<const Swift::metadata *>(type)->nominal_descriptor());
    if (nominal_descriptor == nullptr) {
        return false;
    }

    if (nominal_descriptor->Fields.isNull()) {
        return false;
    }

    if (enum_tag >= nominal_descriptor->getNumPayloadCases()) {
        return false;
    }

    const reflection::FieldRecord &field = nominal_descriptor->Fields->begin().Cur[enum_tag];
    if (!field.hasMangledTypeName()) {
        return false;
    }

    const Metadata *enum_payload_metadata = reinterpret_cast<const Swift::metadata *>(type)->mangled_type_name_ref(field.MangledTypeName.get(), nullptr);
    if (!enum_payload_metadata) {
        return false;
    }

    enum_vwt->destructiveProjectEnumData(value, type);

    OpaqueValue *payload_value = value;
    if (field.isIndirectCase()) {
        auto enum_value_witness_table =
        static_cast<const EnumValueWitnessTable *>(enum_payload_metadata->getValueWitnesses());
        const uint64_t alignmented_mask = enum_value_witness_table->getAlignmentMask();
        const uint64_t aligned_size = (alignmented_mask + 0x10) & (~alignmented_mask);
        payload_value = reinterpret_cast<OpaqueValue *>((*(uintptr_t *)value) + aligned_size);
    }
    body(bodyContext, enum_tag, enum_payload_metadata, payload_value);
    enum_vwt->destructiveInjectEnumTag(value, enum_tag, type);

    return true;
}

extern "C"
bool DanceUITypeApplyMutableEnumData(const Metadata *type,
                                     OpaqueValue *value,
                                     void (*body)(void * SWIFT_CONTEXT,
                                                  int64_t enum_tag,
                                                  const Metadata *type,
                                                  OpaqueValue *value) SWIFT_CC(swift),
                                     void *bodyContext) {
    const swift::ValueWitnessTable *vwt = type->getValueWitnesses();
    if (!vwt->flags.hasEnumWitnesses()) {
        precondition_failure("not an enum type: %s", reinterpret_cast<const Swift::metadata *>(type)->name(false).data);
    }

    const swift::EnumValueWitnessTable *enum_vwt = (const swift::EnumValueWitnessTable *)vwt;
    const unsigned enum_tag = enum_vwt->getEnumTag(value, type);

    const swift::EnumDescriptor *nominal_descriptor = (const swift::EnumDescriptor *)(reinterpret_cast<const Swift::metadata *>(type)->nominal_descriptor());
    if (nominal_descriptor == nullptr) {
        return false;
    }

    if (nominal_descriptor->Fields.isNull()) {
        return false;
    }

    if (enum_tag >= nominal_descriptor->getNumPayloadCases()) {
        return false;
    }

    const reflection::FieldRecord &field = nominal_descriptor->Fields->begin().Cur[enum_tag];
    if (!field.hasMangledTypeName()) {
        return false;
    }

    const Metadata *payload_metadata = reinterpret_cast<const Swift::metadata *>(type)->mangled_type_name_ref(field.MangledTypeName.get(), nullptr);
    if (!payload_metadata) {
        return false;
    }

    OpaqueValue *payload = value;

    enum_vwt->destructiveProjectEnumData(value, type);

    if (field.isIndirectCase()) {
        reinterpret_cast<const Swift::metadata *>(payload_metadata)->copy_on_write_heap_object(*reinterpret_cast<void **>(value));
        auto enum_value_witness_table =
        static_cast<const EnumValueWitnessTable *>(payload_metadata->getValueWitnesses());
        const uint64_t alignmented_mask = enum_value_witness_table->getAlignmentMask();
        const uint64_t aligned_size = (alignmented_mask + 0x10) & (~alignmented_mask);
        payload = reinterpret_cast<OpaqueValue *>((*(uintptr_t *)value) + aligned_size);
    }

    body(bodyContext, enum_tag, payload_metadata, payload);
    enum_vwt->destructiveInjectEnumTag(value, enum_tag, type);

    return true;
}
