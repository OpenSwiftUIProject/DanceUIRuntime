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

#include <swift/Runtime/Metadata.h>
#include <swift/ABI/MetadataValues.h>
#include <DanceUIRuntime/DanceUI_Debug.hpp>
#include <DanceUIRuntime/DanceUIOpaqueValueProjection.hpp>
#include <DanceUIRuntime/DanceUITupleCopyOptions.h>

using namespace swift;

DANCE_UI_INLINE
const bool __DanceUIMetadataIsTuple(const Metadata *type) {
    return type->getKind() == MetadataKind::Tuple;
}

extern "C"
bool DanceUI_is_tuple(const Metadata *type) {
    return __DanceUIMetadataIsTuple(type);
}

extern "C"
uint64_t DanceUI_tuple_getCount(const Metadata *_tupleType) {
    const Metadata *type = _tupleType;
    if (!__DanceUIMetadataIsTuple(type)) {

        return 1;
    }
    const auto tupleType = static_cast<const TupleTypeMetadata *>(type);
    return tupleType->NumElements;
}

extern "C"
uint64_t DanceUI_tuple_getSize(const Metadata *_tupleType) {
    const Metadata *type = _tupleType;
    return type->vw_size();
}

extern "C"
const Metadata *DanceUI_tuple_getElementType(const Metadata *_tupleType,
                                             const uint32_t index) {
    const Metadata *type = _tupleType;
    size_t limitCount = 1;
    const Metadata *elementType = type;
    if (__DanceUIMetadataIsTuple(type)) {
        const auto tupleType = static_cast<const TupleTypeMetadata *>(type);
        limitCount = tupleType->NumElements;
        const auto &element = tupleType->getElement(index);
        elementType = element.Type;
    }
    if (index >= limitCount) {
        DanceUI::precondition_failure("index out of range: %d", index);
    }
    return elementType;
}

extern "C"
uint64_t DanceUI_tuple_getElementSize(const Metadata *_tupleType,
                                      const uint32_t index) {
    const Metadata *type = _tupleType;
    size_t limitCount = 1;
    uint64_t size = 0;
    if (__DanceUIMetadataIsTuple(type)) {
        const auto tupleType = static_cast<const TupleTypeMetadata *>(type);
        limitCount = tupleType->NumElements;
        const auto &element = tupleType->getElement(index);
        size = element.getTypeLayout()->size;
    } else {
        size = type->vw_size();
    }
    if (index >= limitCount) {
        DanceUI::precondition_failure("index out of range: %d", index);
    }
    return size;
}

extern "C"
uint64_t DanceUI_tuple_getElementOffset(const Metadata *_tupleType,
                                        const uint32_t index) {
    const Metadata *type = _tupleType;
    size_t limitCount = 1;
    uint64_t offset = 0;
    if (__DanceUIMetadataIsTuple(type)) {
        const auto tupleType = static_cast<const TupleTypeMetadata *>(type);
        limitCount = tupleType->NumElements;
        const auto &element = tupleType->getElement(index);
        offset = element.Offset;
    }
    if (index >= limitCount) {
        DanceUI::precondition_failure("index out of range: %d", index);
    }
    return offset;
}

extern "C"
uint64_t DanceUI_tuple_getElementOffsetChecked(const Metadata *_tupleType,
                                               const Metadata *elementType,
                                               const uint32_t index) {
    const Metadata *type = _tupleType;
    const Metadata *targetType = type;
    size_t limitCount = 1;
    uint64_t offset = 0;
    if (__DanceUIMetadataIsTuple(type)) {
        const auto tupleType = static_cast<const TupleTypeMetadata *>(type);
        limitCount = tupleType->NumElements;
        const auto &element = tupleType->getElement(index);
        targetType = element.Type;
        offset = element.Offset;
    }
    if (index >= limitCount) {
        DanceUI::precondition_failure("index out of range: %d", index);
    }
    if (elementType != targetType) {
        DanceUI::precondition_failure("element type mismatch");
    }
    return offset;
}

namespace DanceUI {

namespace swift {

OpaqueValue *update(OpaqueValue *dstValue,
                    OpaqueValue *srcValue,
                    const Metadata *type,
                    const DanceUITupleCopyOptions copyOptions) {


    if (copyOptions > DanceUITupleCopyOptionsInitTake) {
        DanceUI::precondition_failure("unknown copy options %d", copyOptions);
    }
    switch (copyOptions) {
        case DanceUITupleCopyOptionsAssignCopy:
            return type->vw_assignWithCopy(dstValue, srcValue);
        case DanceUITupleCopyOptionsInitCopy:
            return type->vw_initializeWithCopy(dstValue, srcValue);
        case DanceUITupleCopyOptionsAssignTake:
            return type->vw_assignWithTake(dstValue, srcValue);
        case DanceUITupleCopyOptionsInitTake:
            return type->vw_initializeWithTake(dstValue, srcValue);
    }
}

}

}

extern "C"
void DanceUI_tuple_getElement(const Metadata *_tupleType,
                              OpaqueValue *tupleValue,
                              const uint32_t index,
                              OpaqueValue *dstValue,
                              DanceUITupleCopyOptions copyOptions,
                              const Metadata *elementType) {
    const Metadata *type = _tupleType;
    const Metadata *targetType = type;
    size_t limitCount = 1;
    const TupleTypeMetadata::Element *element = nullptr;
    OpaqueValue *srcValue = nullptr;
    if (__DanceUIMetadataIsTuple(type)) {
        const auto tupleType = static_cast<const TupleTypeMetadata *>(type);
        limitCount = tupleType->NumElements;
        element = &tupleType->getElements()[index];
        targetType = element->Type;
    }
    if (index >= limitCount) {
        DanceUI::precondition_failure("index out of range: %d", index);
    }
    if (elementType != targetType) {
        DanceUI::precondition_failure("element type mismatch");
    }
    if (element) {
        srcValue = element->findIn(tupleValue);
    } else {
        srcValue = tupleValue;
    }

    DanceUI::swift::update(dstValue, srcValue, elementType, copyOptions);
}

extern "C"
void DanceUI_tuple_setElement(const Metadata *_tupleType,
                              OpaqueValue *tupleValue,
                              const uint32_t index,
                              OpaqueValue *srcValue,
                              const DanceUITupleCopyOptions copyOptions,
                              const Metadata *elementType) {
    const Metadata *type = _tupleType;
    const Metadata *targetType = type;
    size_t limitCount = 1;
    const TupleTypeMetadata::Element *element = nullptr;
    OpaqueValue *dstValue = nullptr;
    if (__DanceUIMetadataIsTuple(type)) {
        const auto tupleType = static_cast<const TupleTypeMetadata *>(type);
        limitCount = tupleType->NumElements;
        element = &tupleType->getElements()[index];
        targetType = element->Type;
    }
    if (index >= limitCount) {
        DanceUI::precondition_failure("index out of range: %d", index);
    }
    if (elementType != targetType) {
        DanceUI::precondition_failure("element type mismatch");
    }
    if (element) {
        dstValue = element->findIn(tupleValue);
    } else {
        dstValue = tupleValue;
    }

    DanceUI::swift::update(dstValue, srcValue, elementType, copyOptions);
}

extern "C"
void DanceUI_tuple_destroyElement(OpaqueValue *value,
                                  const uint32_t index,
                                  const Metadata *_tupleType) {
    const Metadata *type = _tupleType;
    const bool isTuple = __DanceUIMetadataIsTuple(type);
    const auto tupleType = static_cast<const TupleTypeMetadata *>(type);
    if (!((isTuple && index < tupleType->NumElements) ||
          (!isTuple && index == 0))) {
        DanceUI::precondition_failure("index out of range: %d", index);
    }

    OpaqueValue *dstValue = nullptr;
    const Metadata *metadata = nullptr;
    if (isTuple) {
        const auto &element = tupleType->getElement(index);
        dstValue = element.findIn(value);
        metadata = element.Type;
    } else {
        dstValue = value;
        metadata = type;
    }

    const ValueWitnessTable *valueWitnessTable = metadata->getValueWitnesses();
    if (valueWitnessTable->isPOD()) {
        return;
    }
    valueWitnessTable->destroy(dstValue, tupleType);
}

extern "C"
void DanceUI_tuple_destroy(OpaqueValue *tupleValue,
                           const Metadata *type) {
    const ValueWitnessTable *valueWitnessTable = type->getValueWitnesses();
    if (valueWitnessTable->isPOD()) {
        return;
    }
    valueWitnessTable->destroy(tupleValue, type);
}

extern "C"
const Metadata *DanceUI_newTupleType(const unsigned elementCount,
                                     const Metadata *const *elements) {
    const auto response = swift_getTupleTypeMetadata(MetadataState::Complete,
                                                     elementCount, elements,
                                                     nullptr,
                                                     nullptr);
    if (response.State != MetadataState::Complete) {
        DanceUI::precondition_failure("invalid tuple type.");
    }
    return response.Value;
}
