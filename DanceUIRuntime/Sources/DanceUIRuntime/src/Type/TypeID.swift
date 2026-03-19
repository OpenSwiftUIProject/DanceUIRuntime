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

@_silgen_name("DanceUITypeCopyDescription")
@inline(__always)
@inlinable
internal func DanceUITypeCopyDescription(_ type: Any.Type) -> CFString

@_silgen_name("DanceUITypeGetMemberTypeWithOffsetAndTypeSize")
@inline(__always)
@inlinable
internal func DanceUITypeGetMemberTypeWithOffsetAndTypeSize(_ type: Any.Type, _ offset: Int, _ typeSize: Int) -> Any.Type?

#if DEBUG
@_silgen_name("DanceUITypeFetch")
@inline(__always)
@inlinable
internal func DanceUITypeFetch(_ type: Any.Type)
#endif

@frozen
public struct TypeID: Hashable, CustomStringConvertible {

    public let type: Any.Type

    @inlinable
    public init(_ type: Any.Type) {
        self.type = type
    }

    @inlinable
    public var alignmentMask: Int {
        DanceUITypeGetAlignmentMask(self.type)
    }

    @inlinable
    public var size: Int {
        DanceUITypeGetSize(self.type)
    }

    @inlinable
    public var stride: Int {
        DanceUITypeGetStride(self.type)
    }

    @inlinable
    public var isBitwiseTakable: Bool {
        DanceUITypeIsBitwiseTakable(self.type)
    }

    @inlinable
    public func forEachField(options: DGTypeApplyOptions,
                             do body: (UnsafePointer<Int8>, Int, Any.Type) -> Bool) -> Bool {
        applyFields2(type: type, options: options, body: body)
    }

    @inlinable
    public static func == (lhs: TypeID, rhs: TypeID) -> Bool {
        lhs.type == rhs.type
    }

    @inlinable
    public func hash(into hasher: inout Hasher) {
        hasher.combine(ObjectIdentifier(type))
    }

    @inlinable
    public var description: String {
        DanceUITypeCopyDescription(type) as String
    }

    @inlinable
    public func typeOfMember(at offset: Int, size typeSize: Int) -> TypeID? {
        guard let memberType = DanceUITypeGetMemberTypeWithOffsetAndTypeSize(type, offset, typeSize) else {
            return nil
        }
        return TypeID(memberType)
    }

#if DEBUG
    public func fetch() {
        DanceUITypeFetch(type);
    }
#endif

}

@inlinable
public func forEachField(type: Any.Type, do body: (UnsafePointer<Int8>, Int, Any.Type) -> Void) {
    applyFields(type: type, body: body)
}
