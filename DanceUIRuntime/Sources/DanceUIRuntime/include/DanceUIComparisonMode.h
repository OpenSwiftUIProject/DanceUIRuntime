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

#ifndef DanceUIComparisonMode_h
#define DanceUIComparisonMode_h

#include <CoreFoundation/CoreFoundation.h>
#include <DanceUIRuntime/DanceUISwiftSupport.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

DANCE_UI_ASSUME_NONNULL_BEGIN

typedef CF_OPTIONS(uint32_t, DanceUIComparisonMode) {
    DanceUIComparisonModeDefault = 0,
    DanceUIComparisonModePod = 1,
    DanceUIComparisonModeInline = 2,
    DanceUIComparisonModeEquatable = DanceUIComparisonModePod | DanceUIComparisonModeInline,
    DanceUIComparisonModeAsynchronous = 0x100,
    DanceUIComparisonModeSynchronous = 0x200
} SWIFT_NAME(DGComparisonMode);

DANCE_UI_ASSUME_NONNULL_END

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif
