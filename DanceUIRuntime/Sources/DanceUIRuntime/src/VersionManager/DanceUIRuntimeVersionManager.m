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

#import <Foundation/Foundation.h>
#import "DanceUIRuntimeVersionManager.h"

#ifndef DanceUIRuntime_POD_VERSION
#define DanceUIRuntime_POD_VERSION @"(Undefined Version)"
#endif

#ifndef DanceUIRuntime_COMMIT_ID
#define DanceUIRuntime_COMMIT_ID @"(Undefined Commit Id)"
#endif

NSString *const _Nonnull DanceUIRuntimeVersion = DanceUIRuntime_POD_VERSION;
NSString *const _Nonnull DanceUIRuntimeCommitId = DanceUIRuntime_COMMIT_ID;
