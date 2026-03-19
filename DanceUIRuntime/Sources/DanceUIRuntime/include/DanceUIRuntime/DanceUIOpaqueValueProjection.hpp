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

#ifndef DanceUIOpaqueValueProjection_hpp
#define DanceUIOpaqueValueProjection_hpp

#include <DanceUIRuntime/DanceUISwiftSupport.h>
#include <swift/ABI/Metadata.h>

namespace DanceUI {

DANCE_UI_INLINE
const swift::OpaqueValue *projection(const swift::OpaqueValue *value,
                                     size_t offset) {
    auto bytes = reinterpret_cast<const char *>(value);
    return reinterpret_cast<const swift::OpaqueValue *>(bytes + offset);
}

}


#endif
