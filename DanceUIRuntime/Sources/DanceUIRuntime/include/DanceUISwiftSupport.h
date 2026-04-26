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

#ifndef DanceUISwiftSupport_h
#define DanceUISwiftSupport_h


#define SWIFT_CC(CC) SWIFT_CC_##CC


#define SWIFT_CC_c


#if __has_attribute(swiftcall)
#define SWIFT_CC_swift __attribute__((swiftcall))
#define SWIFT_CONTEXT __attribute__((swift_context))
#define SWIFT_ERROR_RESULT __attribute__((swift_error_result))
#define SWIFT_INDIRECT_RESULT __attribute__((swift_indirect_result))
#else
#define SWIFT_CC_swift
#define SWIFT_CONTEXT
#define SWIFT_ERROR_RESULT
#define SWIFT_INDIRECT_RESULT
#endif

#ifndef SWIFT_NAME
#if __has_attribute(swift_name)
#define SWIFT_NAME(_name) __attribute__((swift_name(#_name)))
#else
#define SWIFT_NAME(_name)
#endif
#endif

#ifndef SWIFT_WRAPPER
#if __has_attribute(swift_wrapper)
#define SWIFT_WRAPPER __attribute__((swift_wrapper(struct)))
#else
#define SWIFT_WRAPPER
#endif
#endif

#ifndef SWIFT_NEWTYPE
#if __has_attribute(swift_newtype)
#define SWIFT_NEWTYPE(keyword) __attribute__((swift_newtype(keyword)))
#else
#define SWIFT_NEWTYPE(keyword)
#endif
#endif

#ifndef SWIFT_FLAG_ENUM
#define SWIFT_FLAG_ENUM __attribute__((flag_enum))
#else
#define SWIFT_FLAG_ENUM
#endif

#ifdef DANCE_UI_DISABLE_INLINE_OPTIMIZATION

#define __DANCE_UI_DISABLE_INLINE_OPTIMIZATION__

#if defined(DANCE_UI_INLINE)
#undef DANCE_UI_INLINE
#endif

#define DANCE_UI_INLINE inline __attribute__((noinline))

#else

#define DANCE_UI_INLINE inline __attribute__((always_inline))

#endif

#if __has_feature(assume_nonnull)
#define DANCE_UI_ASSUME_NONNULL_BEGIN _Pragma("clang assume_nonnull begin")
#define DANCE_UI_ASSUME_NONNULL_END   _Pragma("clang assume_nonnull end")
#else
#define DANCE_UI_ASSUME_NONNULL_BEGIN
#define DANCE_UI_ASSUME_NONNULL_END
#endif

#if !__has_feature(nullability)
#ifndef _Nullable
#define _Nullable
#endif
#ifndef _Nonnull
#define _Nonnull
#endif
#ifndef _Null_unspecified
#define _Null_unspecified
#endif
#endif

#if __has_attribute(objc_bridge) && __has_feature(objc_bridge_id) && __has_feature(objc_bridge_id_on_typedefs)

#define DANCE_UI_BRIDGED_TYPE(T)            __attribute__((objc_bridge(T)))
#define DANCE_UI_BRIDGED_MUTABLE_TYPE(T)    __attribute__((objc_bridge_mutable(T)))
#define DANCE_UI_RELATED_TYPE(T,C,I)        __attribute__((objc_bridge_related(T,C,I)))
#else
#define DANCE_UI_BRIDGED_TYPE(T)
#define DANCE_UI_BRIDGED_MUTABLE_TYPE(T)
#define DANCE_UI_RELATED_TYPE(T,C,I)
#endif

#if __has_attribute(enum_extensibility)
#define DANCE_UI_ENUM_ATTRIBUTES __attribute__((enum_extensibility(open)))
#define DANCE_UI_CLOSED_ENUM_ATTRIBUTES __attribute__((enum_extensibility(closed)))
#define DANCE_UI_OPTIONS_ATTRIBUTES __attribute__((flag_enum,enum_extensibility(open)))
#else
#define DANCE_UI_ENUM_ATTRIBUTES
#define DANCE_UI_CLOSED_ENUM_ATTRIBUTES
#define DANCE_UI_OPTIONS_ATTRIBUTES
#endif

#endif
