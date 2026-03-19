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

@frozen
public struct ValueWitnessTable {

    public let typeID: TypeID

    @inlinable
    public var alignmentMask: Int {
        typeID.alignmentMask
    }

    @inlinable
    public var size: Int {
        typeID.size
    }

    @inlinable
    public var stride: Int {
        typeID.stride
    }

    @inlinable
    public var isBitwiseTakable: Bool {
        typeID.isBitwiseTakable
    }

    @inlinable
    public init(_ type: Any.Type) {
        self.init(.init(type))
    }

    @inlinable
    public init(_ typeID: TypeID) {
        self.typeID = typeID
    }

    public func initialize(_ dst: UnsafeMutableRawPointer, _ src: UnsafeRawPointer) -> Void {
        DanceUITypeValueWitnessTableInitialize(dst, src, typeID.type)
    }

    public func moveInitialize(_ dst: UnsafeMutableRawPointer, _ src: UnsafeMutableRawPointer) -> Void {
        DanceUITypeValueWitnessTableMoveInitialize(dst, src, typeID.type)
    }

    public func assign(_ dst: UnsafeMutableRawPointer, _ src: UnsafeRawPointer) -> Void {
        DanceUITypeValueWitnessTableAssign(dst, src, typeID.type)
    }

    public func moveAssign(_ dst: UnsafeMutableRawPointer, _ src: UnsafeMutableRawPointer) -> Void {
        DanceUITypeValueWitnessTableMoveAssign(dst, src, typeID.type)
    }

    public func destroy(_ value: UnsafeMutableRawPointer) -> Void {
        DanceUITypeValueWitnessTableDestroy(value, typeID.type)
    }
}

@frozen
public struct TypedPointer {

    public var type: Any.Type

    public let pointer: UnsafeRawPointer

    @inlinable
    public init(_ type: Any.Type, pointer: UnsafeRawPointer) {
        self.type = type
        self.pointer = pointer
    }

    @inlinable
    public init<T>(_ pointer: UnsafePointer<T>) {
        self.type = T.self
        self.pointer = .init(pointer)
    }
}

@frozen
public struct MutableTypedPointer {

    public var type: Any.Type

    public let pointer: UnsafeMutableRawPointer

    @inlinable
    public init(_ type: Any.Type, pointer: UnsafeMutableRawPointer) {
        self.type = type
        self.pointer = pointer
    }

    @inlinable
    public init<T>(_ pointer: UnsafeMutablePointer<T>) {
        self.type = T.self
        self.pointer = .init(pointer)
    }

    @inlinable
    public func initialize(to ptr: Self) {
        precondition(type == ptr.type)
        DanceUITypeValueWitnessTableInitialize(pointer, ptr.pointer, type)
    }

    @inlinable
    public func initialize(to ptr: TypedPointer) {
        precondition(type == ptr.type)
        DanceUITypeValueWitnessTableInitialize(pointer, ptr.pointer, type)
    }

    @inlinable
    public func moveInitialize(from ptr: Self) {
        precondition(type == ptr.type)
        DanceUITypeValueWitnessTableMoveInitialize(pointer, ptr.pointer, type)
    }

    @inlinable
    public func assign(from ptr: Self) {
        precondition(type == ptr.type)
        DanceUITypeValueWitnessTableAssign(pointer, ptr.pointer, type)
    }

    @inlinable
    public func assign(from ptr: TypedPointer) {
        precondition(type == ptr.type)
        DanceUITypeValueWitnessTableAssign(pointer, ptr.pointer, type)
    }

    @inlinable
    public func moveAssign(from ptr: Self) {
        precondition(type == ptr.type)
        DanceUITypeValueWitnessTableMoveAssign(pointer, ptr.pointer, type)
    }

    @inlinable
    public func destroy() {
        DanceUITypeValueWitnessTableDestroy(pointer, type)
    }

}
