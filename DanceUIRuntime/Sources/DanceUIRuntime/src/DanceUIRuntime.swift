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

import Foundation

@_silgen_name("DanceUI_is_equatable")
@inlinable
@inline(__always)
internal func is_equatable(_ type: Any.Type) -> Bool

@_silgen_name("DanceUI_equatable")
@inlinable
@inline(__always)
internal func equatable<T>(lhs: T, rhs: T) -> Bool

@_silgen_name("DanceUI_equatable")
@inlinable
@inline(__always)
internal func equatable(lhs: UnsafeRawPointer, rhs: UnsafeRawPointer, type: Any.Type) -> Bool

@_silgen_name("DanceUI_is_tuple")
@inlinable
@inline(__always)
internal func is_tuple(_ type: Any.Type) -> Bool

@_silgen_name("DanceUI_tuple_getCount")
@inlinable
@inline(__always)
internal func tuple_getCount(_ tuple: Any.Type) -> Int

@_silgen_name("DanceUI_tuple_getSize")
@inlinable
@inline(__always)
internal func tuple_getSize(_ type: Any.Type) -> Int

@_silgen_name("DanceUI_tuple_getElementType")
@inlinable
@inline(__always)
internal func tuple_getElementType(_ type: Any.Type,
                                   _ index: Int) -> Any.Type

@_silgen_name("DanceUI_tuple_getElementSize")
@inlinable
@inline(__always)
internal func tuple_getElementSize(_ type: Any.Type,
                                   _ index: Int) -> Int

@_silgen_name("DanceUI_tuple_getElementOffset")
@inlinable
@inline(__always)
internal func tuple_getElementOffset(_ type: Any.Type,
                                     _ index: Int) -> Int

@_silgen_name("DanceUI_tuple_getElementOffsetChecked")
@inlinable
@inline(__always)
internal func tuple_getElementOffsetChecked(_ type: Any.Type,
                                            _ element: Any.Type,
                                            _ index: Int) -> Int

@_silgen_name("DanceUI_tuple_getElement")
@inlinable
@inline(__always)
internal func tuple_getElement<R>(_ tupleType: Any.Type,
                                  _ tuple: UnsafeMutableRawPointer,
                                  _ index: Int,
                                  _ result: UnsafeMutablePointer<R>,
                                  _ copyOptions: DGTupleCopyOptions)

@_silgen_name("DanceUI_tuple_setElement")
@inlinable
@inline(__always)
internal func tuple_setElement<R>(_ tupleType: Any.Type,
                                  _ tuple: UnsafeMutableRawPointer,
                                  _ index: Int,
                                  _ value: UnsafePointer<R>,
                                  _ copyOptions: DGTupleCopyOptions)

@_silgen_name("DanceUI_newTupleType")
@inlinable
@inline(__always)
internal func newTupleType(_ count: Int,
                           _ types: UnsafePointer<Any.Type>) -> Any.Type

@_silgen_name("DanceUI_tuple_destroyElement")
@inlinable
@inline(__always)
internal func tuple_destroyElement(_ tuple: UnsafeMutableRawPointer,
                                   _ index: Int,
                                   _ type: Any.Type)

@_silgen_name("DanceUICompareValues")
@inlinable
@inline(__always)
public func DGCompareValues<T>(lhs: T, rhs: T, options: DGComparisonMode = [.equatable, .asynchronous]) -> Bool

@_silgen_name("DanceUICompareValuesWithType")
@inlinable
@inline(__always)
public func DGCompareValues(lhs: UnsafeRawPointer,
                            rhs: UnsafeRawPointer,
                            options: DGComparisonMode = [.equatable, .asynchronous],
                            type: Any.Type) -> Bool

@_silgen_name("DanceUICompareValuesPartial")
@inlinable
@inline(__always)
public func DGCompareValues(lhs: UnsafeRawPointer,
                            rhs: UnsafeRawPointer,
                            offset: Int,
                            size: Int,
                            options: DGComparisonMode = .equatable,
                            type: Any.Type) -> Bool

@_silgen_name("DanceUIApplyFields")
@inlinable
@inline(__always)
public func applyFields(type: Any.Type,
                        body: (UnsafePointer<Int8>, Int, Any.Type) -> ())

@_silgen_name("DanceUIApplyFields2")
@inlinable
@inline(__always)
public func applyFields2(type: Any.Type,
                         options: DGTypeApplyOptions,
                         body: (UnsafePointer<Int8>, Int, Any.Type) -> Bool) -> Bool


@_silgen_name("DanceUITypeGetAlignmentMask")
@inlinable
@inline(__always)
internal func DanceUITypeGetAlignmentMask(_ type: Any.Type) -> Int

@_silgen_name("DanceUITypeGetSize")
@inlinable
@inline(__always)
internal func DanceUITypeGetSize(_ type: Any.Type) -> Int

@_silgen_name("DanceUITypeGetStride")
@inlinable
@inline(__always)
internal func DanceUITypeGetStride(_ type: Any.Type) -> Int

@_silgen_name("DanceUITypeIsBitwiseTakable")
@inlinable
@inline(__always)
internal func DanceUITypeIsBitwiseTakable(_ type: Any.Type) -> Bool

@_silgen_name("DanceUITypeValueWitnessTableInitialize")
@inlinable
@inline(__always)
internal func DanceUITypeValueWitnessTableInitialize(_ dst: UnsafeMutableRawPointer,
                                                     _ src: UnsafeRawPointer,
                                                     _ type: Any.Type) -> Void

@_silgen_name("DanceUITypeValueWitnessTableMoveInitialize")
@inlinable
@inline(__always)
internal func DanceUITypeValueWitnessTableMoveInitialize(_ dst: UnsafeMutableRawPointer,
                                                         _ src: UnsafeMutableRawPointer,
                                                         _ type: Any.Type) -> Void

@_silgen_name("DanceUITypeValueWitnessTableAssign")
@inlinable
@inline(__always)
internal func DanceUITypeValueWitnessTableAssign(_ dst: UnsafeMutableRawPointer,
                                                 _ src: UnsafeRawPointer,
                                                 _ type: Any.Type) -> Void

@_silgen_name("DanceUITypeValueWitnessTableMoveAssign")
@inlinable
@inline(__always)
internal func DanceUITypeValueWitnessTableMoveAssign(_ dst: UnsafeMutableRawPointer,
                                                     _ src: UnsafeMutableRawPointer,
                                                     _ type: Any.Type) -> Void

@_silgen_name("DanceUITypeValueWitnessTableDestroy")
@inlinable
@inline(__always)
internal func DanceUITypeValueWitnessTableDestroy(_ value: UnsafeMutableRawPointer,
                                                  _ type: Any.Type) -> Void
