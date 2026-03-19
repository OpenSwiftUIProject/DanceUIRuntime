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

#include <DanceUIRuntime/DanceUI_Debug.hpp>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(__APPLE__)
#include <asl.h>
#elif defined(__ANDROID__)
#include <android/log.h>
#endif

static char *__DanceUIRuntimeLastPreconditionFailureLog = NULL;

extern "C" const char *DanceUIRuntimeGetLastPreconditionFailureLog(void) {
    return __DanceUIRuntimeLastPreconditionFailureLog;
}

__attribute__((__noreturn__))
__attribute__((__noinline__))
extern "C" const void __DanceUIRuntimeFatalError(void) {
    abort();
}

namespace DanceUI {

__attribute__((__noreturn__))
void precondition_failure(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char *log;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
    vasprintf(&log, format, args);
#pragma GCC diagnostic pop
#if defined(__APPLE__)
    write(STDERR_FILENO, log, strlen(log));
#elif defined(__ANDROID__)
    __android_log_print(ANDROID_LOG_FATAL, "SwiftRuntime", "%s", log);
#endif
    asl_log(nullptr, nullptr, ASL_LEVEL_ERR, "%s", log);
    size_t str_length = strlen(log);
    char *str = (char *)malloc(str_length + 1);
    if (str != nullptr) {
        memset(str, 0, str_length + 1);
        strncpy(str, log, str_length);
    }
    __DanceUIRuntimeLastPreconditionFailureLog = str;
    __DanceUIRuntimeFatalError();
}
}
