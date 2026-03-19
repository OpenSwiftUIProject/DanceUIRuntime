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
#include <swift/Runtime/Casting.h>

#include <DanceUIRuntime/DanceUI_Debug.hpp>
#include <DanceUIRuntime/LayoutDescriptor.hpp>

using namespace swift;
using namespace DanceUI;


extern "C" const ProtocolDescriptor PROTOCOL_DESCR_SYM(SH);
static constexpr auto &HashableProtocolDescriptor = PROTOCOL_DESCR_SYM(SH);

extern "C" bool DanceUI_hashable(const Metadata *type) {
    return swift_conformsToProtocol(type, &HashableProtocolDescriptor);
}


extern "C" const ProtocolDescriptor PROTOCOL_DESCR_SYM(SQ);
static constexpr auto &EquatableProtocolDescriptor = PROTOCOL_DESCR_SYM(SQ);

extern "C" bool DanceUI_is_equatable(const Metadata *type) {
    return swift_conformsToProtocol(type, &EquatableProtocolDescriptor);
}

SWIFT_CC(swift)
extern "C" bool DanceUI_DispatchEquatable(const OpaqueValue *lhs,
                                          const OpaqueValue *rhs,
                                          const Metadata *type,
                                          const WitnessTable *witnessTable);

extern "C" bool DanceUI_equatable(const OpaqueValue *lhs,
                                  const OpaqueValue *rhs,
                                  const Metadata *type) {

    const WitnessTable *witnessTable = swift_conformsToProtocol(type, &EquatableProtocolDescriptor);
    if (!witnessTable) {
        DanceUI::precondition_failure("type is not equatable");
    }
    return DanceUI_DispatchEquatable(lhs, rhs, type, witnessTable);
}
