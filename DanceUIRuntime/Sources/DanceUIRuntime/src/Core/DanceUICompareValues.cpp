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

#include <DanceUIRuntime/metadata_visitor.hpp>
#include <DanceUIRuntime/LayoutDescriptor.hpp>
#include <DanceUIRuntime/DanceUIEquatable.hpp>

using namespace swift;
using namespace DanceUI;

CF_EXTERN_C_BEGIN

CF_EXPORT
bool DanceUICompareValues(const OpaqueValue *lhs,
                          const OpaqueValue *rhs,
                          const DanceUIComparisonMode comparisonMode,
                          const Metadata *type) {
    const auto &layoutDescriptor = LayoutDescriptor::fetch(reinterpret_cast<const Swift::metadata *>(type),
                                                           comparisonMode);
    if (layoutDescriptor == LayoutDescriptor::get_invalid()) {
        return compare_bytes(lhs, rhs, type->vw_size());
    }
    return layoutDescriptor.compare(lhs,
                                    rhs,
                                    comparisonMode,
                                    LayoutDescriptor::HeapMode::HeapMode_0);
}

CF_EXPORT
bool DanceUICompareValuesWithType(const OpaqueValue *lhs,
                                  const OpaqueValue *rhs,
                                  const DanceUIComparisonMode comparisonMode,
                                  const Metadata *type) {
    return DanceUICompareValues(lhs, rhs, comparisonMode, type);
}

CF_EXPORT
bool DanceUICompareValuesPartial(const OpaqueValue *lhs,
                                 const OpaqueValue *rhs,
                                 size_t offset,
                                 size_t size,
                                 const DanceUIComparisonMode comparisonMode,
                                 const Metadata *type) {
    const auto &layout_descriptor = LayoutDescriptor::fetch(reinterpret_cast<const Swift::metadata *>(type),
                                                            comparisonMode);
    if (layout_descriptor == LayoutDescriptor::get_invalid()) {
        return compare_bytes(reinterpret_cast<const OpaqueValue *>(reinterpret_cast<uintptr_t>(lhs) + offset),
                             reinterpret_cast<const OpaqueValue *>(reinterpret_cast<uintptr_t>(rhs) + offset),
                             size);
    }
    return layout_descriptor.compare_partial(lhs,
                                             rhs,
                                             size_t(offset),
                                             size,
                                             comparisonMode,
                                             LayoutDescriptor::HeapMode::HeapMode_0);
}

#if DEBUG
CF_EXPORT
void DanceUITypeFetch(const void *type) {
    const auto &layoutDescriptor = LayoutDescriptor::fetch(reinterpret_cast<const Swift::metadata *>(type),
                                                           DanceUIComparisonModeEquatable | DanceUIComparisonModeSynchronous);
}
#endif

CF_EXPORT
void DanceUIDumpType(const Metadata *type) {
    const auto &layoutDescriptor = LayoutDescriptor::fetch(reinterpret_cast<const Swift::metadata *>(type),
                                                           DanceUIComparisonMode(DanceUIComparisonModeEquatable | DanceUIComparisonModeSynchronous));
    layoutDescriptor.dump();
    return;
}

CF_EXTERN_C_END
