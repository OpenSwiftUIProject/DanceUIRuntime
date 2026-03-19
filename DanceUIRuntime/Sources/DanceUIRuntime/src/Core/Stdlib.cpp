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
#include <swift/Demangling/ManglingMacros.h>

using namespace swift;

namespace llvm {
int EnableABIBreakingChecks = 0;
};


SWIFT_CC(swift) SWIFT_RUNTIME_STDLIB_INTERNAL
void _swift_makeAnyHashableUsingDefaultRepresentation(
  const OpaqueValue *value,
  const void *anyHashableResultPointer,
  const Metadata *T,
  const WitnessTable *hashableWT
) {
  abort();
}


SWIFT_CC(swift) SWIFT_RUNTIME_STDLIB_INTERNAL
bool _swift_stdlib_Hashable_isEqual_indirect(
  const void *lhsValue, const void *rhsValue, const Metadata *type,
  const void *wt) {
  abort();
}

SWIFT_CC(swift) SWIFT_RUNTIME_STDLIB_INTERNAL
intptr_t _swift_stdlib_Hashable_hashValue_indirect(
  const void *value, const Metadata *type, const void *wt) {
  abort();
}

SWIFT_CC(swift) SWIFT_RUNTIME_STDLIB_INTERNAL
void _swift_convertToAnyHashableIndirect(
  OpaqueValue *source, OpaqueValue *destination, const Metadata *sourceType,
  const void *sourceConformance) {
  abort();
}

SWIFT_CC(swift) SWIFT_RUNTIME_STDLIB_INTERNAL
bool _swift_anyHashableDownCastConditionalIndirect(
  OpaqueValue *source, OpaqueValue *destination, const Metadata *targetType) {
  abort();
}


SWIFT_CC(swift) SWIFT_RUNTIME_STDLIB_INTERNAL
void _swift_arrayDownCastIndirect(OpaqueValue *destination,
                                  OpaqueValue *source,
                                  const Metadata *sourceValueType,
                                  const Metadata *targetValueType) {
  abort();
}

SWIFT_CC(swift) SWIFT_RUNTIME_STDLIB_INTERNAL
bool _swift_arrayDownCastConditionalIndirect(OpaqueValue *destination,
                                             OpaqueValue *source,
                                             const Metadata *sourceValueType,
                                             const Metadata *targetValueType) {
  abort();
}

SWIFT_CC(swift) SWIFT_RUNTIME_STDLIB_INTERNAL
void _swift_setDownCastIndirect(OpaqueValue *destination,
                                OpaqueValue *source,
                                const Metadata *sourceValueType,
                                const Metadata *targetValueType,
                                const void *sourceValueHashable,
                                const void *targetValueHashable) {
  abort();
}

SWIFT_CC(swift) SWIFT_RUNTIME_STDLIB_INTERNAL
bool _swift_setDownCastConditionalIndirect(OpaqueValue *destination,
                                           OpaqueValue *source,
                                           const Metadata *sourceValueType,
                                           const Metadata *targetValueType,
                                           const void *sourceValueHashable,
                                           const void *targetValueHashable) {
  abort();
}

SWIFT_CC(swift) SWIFT_RUNTIME_STDLIB_INTERNAL
void _swift_dictionaryDownCastIndirect(OpaqueValue *destination,
                                       OpaqueValue *source,
                                       const Metadata *sourceKeyType,
                                       const Metadata *sourceValueType,
                                       const Metadata *targetKeyType,
                                       const Metadata *targetValueType,
                                       const void *sourceKeyHashable,
                                       const void *targetKeyHashable) {
  abort();
}

SWIFT_CC(swift) SWIFT_RUNTIME_STDLIB_INTERNAL
bool _swift_dictionaryDownCastConditionalIndirect(OpaqueValue *destination,
                                        OpaqueValue *source,
                                        const Metadata *sourceKeyType,
                                        const Metadata *sourceValueType,
                                        const Metadata *targetKeyType,
                                        const Metadata *targetValueType,
                                        const void *sourceKeyHashable,
                                        const void *targetKeyHashable) {
  abort();
}

SWIFT_CC(swift) SWIFT_RUNTIME_STDLIB_INTERNAL
void _bridgeNonVerbatimBoxedValue(const OpaqueValue *sourceValue,
                                  OpaqueValue *destValue,
                                  const Metadata *nativeType) {
  abort();
}

SWIFT_CC(swift) SWIFT_RUNTIME_STDLIB_INTERNAL
void _bridgeNonVerbatimFromObjectiveCToAny(HeapObject *sourceValue,
                                           OpaqueValue *destValue) {
  abort();
}


SWIFT_CC(swift) SWIFT_RUNTIME_STDLIB_INTERNAL
int $ss13_getErrorCodeySiSPyxGs0B0RzlF(void *) {
  abort();
}

SWIFT_CC(swift) SWIFT_RUNTIME_STDLIB_INTERNAL
void *$ss23_getErrorDomainNSStringyyXlSPyxGs0B0RzlF(void *) {
  abort();
}

SWIFT_CC(swift) SWIFT_RUNTIME_STDLIB_INTERNAL
void *$ss29_getErrorUserInfoNSDictionaryyyXlSgSPyxGs0B0RzlF(void *) {
  abort();
}

SWIFT_CC(swift) SWIFT_RUNTIME_STDLIB_INTERNAL
void *$ss32_getErrorEmbeddedNSErrorIndirectyyXlSgSPyxGs0B0RzlF(void *) {
  abort();
}
