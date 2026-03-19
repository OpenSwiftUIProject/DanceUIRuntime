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

import XCTest
import DanceUIRuntime

private struct Unary<ChildType> {

    var child: ChildType

}

private struct Binary<FirstType, LastType> {

    var first: FirstType

    var last: LastType

}

private struct Ternary<FirstType, MidType, LastType> {

    var first: FirstType

    var mid: MidType

    var last: LastType

}

class DanceUITypeUnitTests: XCTestCase {


    func testCannotDisambiguateZeroOffsetMemberTypeAndEqualSizedParentType() {
        let value: String = "Hello, world!"

        let unary = Unary(child: value)

        let typeOfMember1 = TypeID(type(of: unary)).typeOfMember(at: 0, size: MemoryLayout.size(ofValue: value))

        XCTAssertEqual(typeOfMember1, TypeID(type(of: unary)))

        let unaryUnary = Unary(child: unary)

        let typeOfMember2 = TypeID(type(of: unaryUnary)).typeOfMember(at: 0, size: MemoryLayout.size(ofValue: value))

        XCTAssertEqual(typeOfMember2, TypeID(type(of: unaryUnary)))
    }

    func testCanDisambiguateMemberTypeAndNonEqualSizedParentType() {
        let first: Int = 0xa
        let last: String = "Hello, world!"

        let binary = Binary(first: first, last: last)

        withTestingEnvironment(for: binary) { layout in
            let typeOfFirstMember = TypeID(type(of: binary)).typeOfMember(at: layout.offset(of: \.first), size: MemoryLayout.size(ofValue: first))
            let typeOfLastMember = TypeID(type(of: binary)).typeOfMember(at: layout.offset(of: \.last), size: MemoryLayout.size(ofValue: last))
            XCTAssertEqual(typeOfFirstMember, TypeID(typeOf: first))
            XCTAssertEqual(typeOfLastMember, TypeID(typeOf: last))
        }

    }

    func testCanDisambiguateMemberTypeAndNonEqualSizedParentType2() {
        let first: String = "Hello, world!"
        let last: Int = 0xa

        let binary = Binary(first: first, last: last)

        withTestingEnvironment(for: binary) { layout in
            let typeOfFirstMember = TypeID(type(of: binary)).typeOfMember(at: layout.offset(of: \.first), size: MemoryLayout.size(ofValue: first))
            let typeOfLastMember = TypeID(type(of: binary)).typeOfMember(at: layout.offset(of: \.last), size: MemoryLayout.size(ofValue: last))
            XCTAssertEqual(typeOfFirstMember, TypeID(typeOf: first))
            XCTAssertEqual(typeOfLastMember, TypeID(typeOf: last))
        }
    }

    @_optimize(speed)
    func testCanDisambiguateMemberTypeWithAlignmentPaddingAndNonEqualSizedParentType() {
        let first: String = "Hello, world!"
        let last: Bool = true

        let binary = Binary(first: first, last: last)

        withTestingEnvironment(for: binary) { layout in
            let typeOfFirstMember = TypeID(type(of: binary)).typeOfMember(at: layout.offset(of: \.first), size: MemoryLayout.size(ofValue: first))
            let typeOfLastMember = TypeID(type(of: binary)).typeOfMember(at: layout.offset(of: \.last), size: MemoryLayout.size(ofValue: last))
            XCTAssertEqual(typeOfFirstMember, TypeID(typeOf: first))
            XCTAssertEqual(typeOfLastMember, TypeID(typeOf: last))
        }
    }

    @_optimize(speed)
    func testCanDisambiguateMemberTypeWithAlignmentPaddingAndNonEqualSizedParentType2() {
        let first: Bool = true
        let last: String = "Hello, world!"

        XCTAssertEqual(MemoryLayout.size(ofValue: first), 1)

        let binary = Binary(first: first, last: last)

        withTestingEnvironment(for: binary) { layout in
            let typeOfFirstMember = TypeID(type(of: binary)).typeOfMember(at: layout.offset(of: \.first), size: MemoryLayout.size(ofValue: first))
            let typeOfLastMember = TypeID(type(of: binary)).typeOfMember(at: layout.offset(of: \.last), size: MemoryLayout.size(ofValue: last))
            XCTAssertEqual(typeOfFirstMember, TypeID(typeOf: first))
            XCTAssertEqual(typeOfLastMember, TypeID(typeOf: last))
        }
    }

    func testCannotDisambiguateNonZeroSizedMemberTypeAndEqualSizedParentTypeWhenThereAreVoidMembers() {
        let first: String = "Hello, world!"
        let last: Void = Void()

        let binary = Binary(first: first, last: last)

        withTestingEnvironment(for: binary) { layout in
            let typeOfFirstMember = TypeID(type(of: binary)).typeOfMember(at: layout.offset(of: \.first), size: MemoryLayout.size(ofValue: first))
            let typeOfLastMember = TypeID.typeOfMember(in: binary, at: .offset({.of(&$0.last)}), size: MemoryLayout.size(ofValue: last))
            XCTAssertEqual(typeOfFirstMember, TypeID(typeOf: binary))
            XCTAssertEqual(typeOfLastMember, nil)
        }
    }

    func testCannotDisambiguateNonZeroSizedMemberTypeAndEqualSizedParentTypeWhenThereAreVoidMembers2() {
        let first: Void = Void()
        let last: String = "Hello, world!"

        let binary = Binary(first: first, last: last)

        withTestingEnvironment(for: binary) { layout in
            let typeOfFirstMember = TypeID(type(of: binary)).typeOfMember(at: layout.offset(of: \.first), size: MemoryLayout.size(ofValue: first))
            let typeOfLastMember = TypeID(type(of: binary)).typeOfMember(at: layout.offset(of: \.last), size: MemoryLayout.size(ofValue: last))
            XCTAssertEqual(typeOfFirstMember, nil)
            XCTAssertEqual(typeOfLastMember, TypeID(typeOf: binary))
        }
    }

    func testCanDisambiguateMemberTypesWithIntermeidateZeroSizedMemberType() {
        let first: Int = 0xa
        let mid: Void = Void()
        let last: String = "Hello, world!"

        let ternary = Ternary(first: first, mid: mid, last: last)

        withTestingEnvironment(for: ternary) { layout in
            let typeOfFirstMember = TypeID(type(of: ternary)).typeOfMember(at: layout.offset(of: \.first), size: MemoryLayout.size(ofValue: first))
            let typeOfMidMember = TypeID(type(of: ternary)).typeOfMember(at: layout.offset(of: \.mid), size: MemoryLayout.size(ofValue: mid))
            let typeOfLastMember = TypeID(type(of: ternary)).typeOfMember(at: layout.offset(of: \.last), size: MemoryLayout.size(ofValue: last))
            XCTAssertEqual(typeOfFirstMember, TypeID(typeOf: first))
            XCTAssertEqual(typeOfMidMember, nil)
            XCTAssertEqual(typeOfLastMember, TypeID(typeOf: last))
        }
    }

    func testCanDisambiguateNestedMemberTypes() {
        let first: Int = 0xa
        let mid: Bool = true
        let last: String = "Hello, world!"

        let binary = Binary(first: first, last: mid)

        let ternary = Binary(first: binary, last: last)

        withTestingEnvironment(for: ternary) { layout in
            let typeOfFirstMember = TypeID(type(of: ternary)).typeOfMember(at: layout.offset(of: \.first.first), size: MemoryLayout.size(ofValue: first))
            let typeOfMidMember = TypeID(type(of: ternary)).typeOfMember(at: layout.offset(of: \.first.last), size: MemoryLayout.size(ofValue: mid))
            let typeOfLastMember = TypeID(type(of: ternary)).typeOfMember(at: layout.offset(of: \.last), size: MemoryLayout.size(ofValue: last))
            XCTAssertEqual(typeOfFirstMember, TypeID(typeOf: first))
            XCTAssertEqual(typeOfMidMember, TypeID(typeOf: mid))
            XCTAssertEqual(typeOfLastMember, TypeID(typeOf: last))
        }
    }

    func testUndefinedBehaviorForFunction() {
        let function: () -> Void = {

        }
        let result = TypeID(type(of: function)).typeOfMember(at: 0, size: MemoryLayout.size(ofValue: function))
        XCTAssertNil(result)
    }

    func testUndefinedBehaviorForObjCClass() {
        let classInstance = NSObject()
        let result = TypeID(type(of: classInstance)).typeOfMember(at: 0, size: MemoryLayout.size(ofValue: classInstance))
        XCTAssertNil(result)
    }

    func testUndefinedBehaviorForClass() {
        class MyClass {

        }
        let classInstance = MyClass()
        let result = TypeID(type(of: classInstance)).typeOfMember(at: 0, size: MemoryLayout.size(ofValue: classInstance))
        XCTAssertNil(result)
    }

    func testUndefinedBehaviorForEnum() {
        enum MyEnum {
            case case1
            case case2(Bool)
        }
        let enumCase = MyEnum.case1
        let result = TypeID(type(of: enumCase)).typeOfMember(at: 0, size: MemoryLayout.size(ofValue: enumCase))
        XCTAssertNil(result)
    }

    func testUndefinedBehaviorForExistential() {
        protocol Foo {

        }
        struct Bar: Foo {

        }
        let existential: Foo = Bar()
        let result = TypeID(type(of: existential)).typeOfMember(at: 0, size: MemoryLayout.size(ofValue: existential))
        XCTAssertNil(result)
    }

    func testUndefinedBehaviorForMetaType() {
        struct Foo {

        }
        let metaType: Any.Type = Foo.self
        let result = TypeID(type(of: metaType)).typeOfMember(at: 0, size: MemoryLayout.size(ofValue: metaType))
        XCTAssertNil(result)
    }

    private func withTestingEnvironment<ValueType>(for value: ValueType, do body: (_ layout: MemoryLayout<ValueType>.Type) -> Void) {
        body(MemoryLayout<ValueType>.self)
    }
}

extension TypeID {

    fileprivate init<ValueType>(typeOf value: ValueType) {
        self.init(Swift.type(of: value))
    }

}

extension MemoryLayout {

    fileprivate static func offset(of key: PartialKeyPath<T>) -> Int {
        guard let offset =  offset(of: key) as Int? else {
            preconditionFailure()
        }
        return offset
    }

}


fileprivate struct PointerOffset<Base, Member> {

    fileprivate var byteOffset: Int

    @inline(__always)
    fileprivate init(byteOffset: Int) {
        self.byteOffset = byteOffset
    }

    @inline(__always)
    fileprivate static func invalidScenePointer() -> UnsafeMutablePointer<Base> {
        UnsafeMutableRawPointer(bitPattern: MemoryLayout<Base>.stride)!.assumingMemoryBound(to: Base.self)
    }

    @inline(__always)
    fileprivate static func offset(_ body: (inout Base) -> Self) -> Self {
        guard MemoryLayout<Self>.size != 0 else {
            return Self(byteOffset: 0)
        }
        return body(&UnsafeMutableRawPointer(bitPattern: MemoryLayout<Base>.stride)!.assumingMemoryBound(to: Base.self).pointee)
    }

    @inline(__always)
    fileprivate static func of(_ member: inout Member) -> Self {
        withUnsafePointer(to: &member) { ptr in
            Self(byteOffset: Int(bitPattern: ptr) &- MemoryLayout<Base>.stride)
        }
    }

    @inline(__always)
    fileprivate static func + <A>(_ lhs: PointerOffset<A, Base>, _ rhs: PointerOffset<Base, Member>) -> PointerOffset<A, Member> {
        PointerOffset<A, Member>(byteOffset: lhs.byteOffset &+ rhs.byteOffset)
    }

}

extension PointerOffset where Base == Member {

    @inline(__always)
    fileprivate init() {
        byteOffset = 0
    }
}

extension UnsafePointer {

    @inline(__always)
    fileprivate subscript<Base, Member>(offset: PointerOffset<Base, Member>) -> Member {
        nonmutating unsafeMutableAddress {
            UnsafeMutableRawPointer(mutating: self).advanced(by: offset.byteOffset).assumingMemoryBound(to: Member.self)
        }

        unsafeAddress {
            UnsafeRawPointer(self).advanced(by: offset.byteOffset).assumingMemoryBound(to: Member.self)
        }
    }

    @inline(__always)
    fileprivate static func + <Member>(_ lhs: Self, _ rhs: PointerOffset<Pointee, Member>) -> UnsafePointer<Member> {
        UnsafeRawPointer(lhs).advanced(by: rhs.byteOffset).assumingMemoryBound(to: Member.self)
    }
}

extension UnsafeMutablePointer {

    @inline(__always)
    fileprivate subscript<Base, Member>(offset: PointerOffset<Base, Member>) -> Member {
        nonmutating unsafeMutableAddress {
            UnsafeMutableRawPointer(mutating: self).advanced(by: offset.byteOffset).assumingMemoryBound(to: Member.self)
        }

        unsafeAddress {
            UnsafeRawPointer(self).advanced(by: offset.byteOffset).assumingMemoryBound(to: Member.self)
        }
    }

    @inline(__always)
    fileprivate static func + <Member>(_ lhs: Self, _ rhs: PointerOffset<Pointee, Member>) -> UnsafeMutablePointer<Member> {
        UnsafeMutableRawPointer(mutating: lhs).advanced(by: rhs.byteOffset).assumingMemoryBound(to: Member.self)
    }

}

extension TypeID {

    fileprivate static func typeOfMember<RootType, MemberType>(in rootValue: RootType, at pointerOffset: PointerOffset<RootType, MemberType>, size typeSize: Int) -> TypeID? {
        return TypeID(Swift.type(of: rootValue)).typeOfMember(at: pointerOffset.byteOffset, size: typeSize)
    }

}
