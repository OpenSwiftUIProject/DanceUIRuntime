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
import Combine

fileprivate enum SimpleEnumValue {
    case e0
    case e1
    case e2
    case e3
}

fileprivate enum SinglePayloadEnumValue {
    case e0(Int)
}

fileprivate enum SimpleSingleEnumValue {
    case e0
}

fileprivate enum PayloadEnumValue {
    case e0(Int)
    case e1(Double)
    case e2(String)
    case e3([Bool])
    case e4
}

fileprivate indirect enum IndirectEnumValue {
    case add(IndirectEnumValue, IndirectEnumValue)
    case value(Int)
}

struct Font: Equatable {

    struct Weight: Equatable {
        fileprivate let _rawWeight: CGFloat
    }

    public enum TextStyle : Hashable, CaseIterable {

        case largeTitle

        case title

        case headline

        case subheadline

        case body

        case callout

        case footnote

        case caption

    }


    public enum Design : Hashable {

        case `default`

        @available(watchOS, unavailable)
        case serif

        case rounded

        @available(watchOS, unavailable)
        case monospaced

    }
}

fileprivate struct Text {

    class AnyTextModifier {
        func isEqual(to rhs: AnyTextModifier) -> Bool {
            return self === rhs
        }
    }

    class AnyTextStorage: NSObject {
    }

    enum Storage: Equatable {
        @usableFromInline
        static func == (lhs: Text.Storage, rhs: Text.Storage) -> Bool {
            switch (lhs, rhs) {
            case (.string(let lv), .string(let rv)):
                return lv == rv
            case (.anyTextStorage(let lv), .anyTextStorage(let rv)):
                return lv.isEqual(rv)
            default:
                return false
            }
        }

        case anyTextStorage(_: AnyTextStorage)
        case string(_ :String)
    }

    enum Modifier: Equatable {
        case foregroundColor(_ : Int?)
        case anyTextModifier(_ : AnyTextModifier)
        case font(_: Font?)
        case fontWeight(_: Font.Weight)
        case italic
        case kerning(_: CGFloat)
        case tracking(_: CGFloat)
        case baselineOffset(_: CGFloat)

        @usableFromInline
        static func == (lhs: Modifier, rhs: Modifier) -> Bool {
            switch (lhs, rhs) {
            case (.foregroundColor(let value0), .foregroundColor(let value1)):
                return value0 == value1
            case (.anyTextModifier(let value0), .anyTextModifier(let value1)):
                return value0.isEqual(to: value1)
            case (.font(let value0), .font(let value1)):
                return value0 == value1
            case (.fontWeight(let value0), .fontWeight(let value1)):
                return value0 == value1
            case (.kerning(let value0), .kerning(let value1)):
                return value0 == value1
            case (.tracking(let value0), .tracking(let value1)):
                return value0 == value1
            case (.baselineOffset(let value0), .baselineOffset(let value1)):
                return value0 == value1
            case (.italic, italic):
                return true
            default:
                return false
            }
        }
    }

    let modifier: [Modifier]
    let storage: Storage
}

fileprivate enum NotEquatableMultiPayloadEnum {
    case void
    case int(Int)
    case double(Double)
    case string(String)
    case float(Float)
}

@_silgen_name("AGCompareValues")
@inlinable
@inline(__always)
public func AGCompareValues<T>(lhs: T, rhs: T, options: Int) -> Bool

class DanceUICompareValuesTests: XCTestCase {

    func testBuiltin_int_equal() throws {
        XCTAssertTrue(DGCompareValues(lhs: 0, rhs: 0))
    }

    func testBuiltin_int_notEqual_1() throws {
        XCTAssertFalse(DGCompareValues(lhs: 0, rhs: 1))
    }

    func testBuiltin_int_notEqual_2() throws {
        XCTAssertFalse(DGCompareValues(lhs: 1, rhs: 0))
    }

    func testBuiltin_tuple_int_equal() throws {
        XCTAssertTrue(DGCompareValues(lhs: (0), rhs: (0)))
    }

    func testBuiltin_tuple_int_notEqual_1() throws {
        XCTAssertFalse(DGCompareValues(lhs: (1), rhs: (0)))
    }

    func testBuiltin_tuple_int_notEqual_2() throws {
        XCTAssertFalse(DGCompareValues(lhs: (0), rhs: (1)))
    }

    func testBuiltin_tuple_empty_equal() throws {
        XCTAssertTrue(DGCompareValues(lhs: (), rhs: ()))
    }

    func testBuiltin_tuple_empty_tuple_equal() throws {
        XCTAssertTrue(DGCompareValues(lhs: (()), rhs: (())))
    }


    func testSimpleSingleEnumValue_equal() throws {
        XCTAssertTrue(DGCompareValues(lhs: SimpleSingleEnumValue.e0,
                                      rhs: .e0))
    }

    func testSimpleEnumValue_equal() throws {
        let lhs = SimpleEnumValue.e0
        let rhs = SimpleEnumValue.e0
        XCTAssertTrue(DGCompareValues(lhs: lhs, rhs: rhs))
    }

    func testSimpleEnumValue_notEqual() throws {
        let lhs = SimpleEnumValue.e0
        let rhs = SimpleEnumValue.e1
        XCTAssertFalse(DGCompareValues(lhs: lhs, rhs: rhs))
    }

    func testSinglePayloadEnumValue_equal() throws {
        let lhs = SinglePayloadEnumValue.e0(#line)
        let rhs = lhs
        XCTAssertTrue(DGCompareValues(lhs: lhs, rhs: rhs))
        XCTAssertTrue(DGCompareValues(lhs: lhs, rhs: rhs))
    }

    func testSinglePayloadEnumValue_notEqual() throws {
        let lhs = SinglePayloadEnumValue.e0(#line)
        let rhs = SinglePayloadEnumValue.e0(#line)
        XCTAssertFalse(DGCompareValues(lhs: lhs, rhs: rhs))
    }

    func testPayloadEnumValue_sameEnum_equal() throws {
        let lhs = PayloadEnumValue.e0(#line)
        let rhs = lhs
        XCTAssertTrue(DGCompareValues(lhs: lhs, rhs: rhs))
    }

    func testPayloadEnumValue_sameEnum_notEqual() throws {
        let lhs = PayloadEnumValue.e0(#line)
        let rhs = PayloadEnumValue.e0(#line)
        print("[\(#function)] \(lhs) vs \(rhs)")
        XCTAssertFalse(DGCompareValues(lhs: lhs, rhs: rhs))
    }

    func testPayloadEnumValue_notSameEnum_notEqual() throws {
        let lhs = PayloadEnumValue.e0(#line)
        let rhs = PayloadEnumValue.e1(#line)
        print("[\(#function)] \(lhs) vs \(rhs)")
        XCTAssertFalse(DGCompareValues(lhs: lhs, rhs: rhs))
    }

    func testPayloadEnumValue_notSameEnum_notEqual_1() throws {
        let lhs = PayloadEnumValue.e0(#line)
        let rhs = PayloadEnumValue.e2(#file)
        print("[\(#function)] \(lhs) vs \(rhs)")
        XCTAssertFalse(DGCompareValues(lhs: lhs, rhs: rhs))
    }

    func testPayloadEnumValue_notSameEnum_notEqual_2() throws {
        let lhs = PayloadEnumValue.e4
        let rhs = PayloadEnumValue.e2(#file)
        XCTAssertFalse(DGCompareValues(lhs: lhs, rhs: rhs))
    }

    func testIndirectEnumValue_sameEnum_equal() throws {
        let lhs = IndirectEnumValue.value(0x12345678)
        let rhs = IndirectEnumValue.value(0x12345678)
        XCTAssertTrue(DGCompareValues(lhs: lhs, rhs: rhs))
    }

    func testIndirectEnumValue_sameEnum_equal_1() throws {
        let lhs = IndirectEnumValue.add(.add(.value(1), .value(2)),
                                        .add(.value(3), .value(4)))
        let rhs = IndirectEnumValue.add(.add(.value(1), .value(2)),
                                        .add(.value(3), .value(4)))
        XCTAssertTrue(DGCompareValues(lhs: lhs, rhs: rhs))
    }

    func testIndirectEnumValue_sameEnum_notEqual() throws {
        let lhs = IndirectEnumValue.value(123)
        let rhs = IndirectEnumValue.value(124)
        XCTAssertFalse(DGCompareValues(lhs: lhs, rhs: rhs))
    }

    func testIndirectEnumValue_notSameEnum_notEqual() throws {
        let lhs = IndirectEnumValue.value(123)
        let rhs = IndirectEnumValue.add(.add(.value(1), .value(2)),
                                        .add(.value(3), .value(4)))
        XCTAssertFalse(DGCompareValues(lhs: lhs, rhs: rhs))
    }

    func testIndirectEnumValue_notSameEnum_notEqual_1() throws {
        let lhs = IndirectEnumValue.add(.add(.value(1), .value(3)),
                                        .add(.value(3), .value(4)))
        let rhs = IndirectEnumValue.add(.add(.value(1), .value(2)),
                                        .add(.value(3), .value(4)))
        XCTAssertFalse(DGCompareValues(lhs: lhs, rhs: rhs))
    }

    func test_Text_same() throws {
        let lhs = Text(modifier: [], storage: .string("hello"))
        let rhs = Text(modifier: [], storage: .string("hello"))
        XCTAssertTrue(DGCompareValues(lhs: lhs, rhs: rhs))
    }

    func test_Text_notSame() throws {
        let lhs = Text(modifier: [.foregroundColor(123)], storage: .string("hello"))
        let rhs = Text(modifier: [.foregroundColor(124)], storage: .string("hello"))
        XCTAssertFalse(DGCompareValues(lhs: lhs, rhs: rhs))
    }

    func test_Array_NotEquatableMultiPayloadEnum_same() throws {
        let lhs: [NotEquatableMultiPayloadEnum] = [.double(123), .int(234)]
        let rhs: [NotEquatableMultiPayloadEnum] = [.double(123), .int(234)]

        XCTAssertFalse(DGCompareValues(lhs: lhs, rhs: rhs))
    }

    func test_Void() throws {

        XCTAssertTrue(DGCompareValues(lhs: (), rhs: ()))
    }

    @available(iOS 13.0, *)
    func test_subscribe() throws {
        let subscribe0 = Timer.publish(every: 1, on: .main, in: .common).autoconnect()
        let subscribe1 = subscribe0
        XCTAssertTrue(DGCompareValues(lhs: subscribe0, rhs: subscribe1))
    }


    func testPartial_equal_offsetAtZero() {
        let lhs = CGRect(origin: CGPoint(x: 1.0, y: 3.0), size: CGSize(width: 5.0, height: 7.0))
        let rhs = CGRect(origin: CGPoint(x: 1.0, y: 3.0), size: CGSize(width: 5.0, height: 7.0))

        withUnsafePointer(to: lhs) { lhsPtr in
            withUnsafePointer(to: rhs) { rhsPtr in
                XCTAssertTrue(DGCompareValues(lhs: lhsPtr, rhs: rhsPtr,
                                              offset: MemoryLayout<CGRect>.offset(of: \.origin)!,
                                              size: MemoryLayout<CGPoint>.size,
                                              options: .equatable,
                                              type: CGRect.self))
            }
        }

    }

    func testPartial_equal_offsetAtZero_partial_equals() {
        let lhs = CGRect(origin: CGPoint(x: 1.0, y: 3.0), size: CGSize(width: 7.0, height: 5.0))
        let rhs = CGRect(origin: CGPoint(x: 1.0, y: 3.0), size: CGSize(width: 5.0, height: 7.0))

        withUnsafePointer(to: lhs) { lhsPtr in
            withUnsafePointer(to: rhs) { rhsPtr in
                XCTAssertTrue(DGCompareValues(lhs: lhsPtr, rhs: rhsPtr,
                                              offset: MemoryLayout<CGRect>.offset(of: \.origin)!,
                                              size: MemoryLayout<CGPoint>.size,
                                              options: .equatable,
                                              type: CGRect.self))
            }
        }

    }

    func testPartial_equal_offsetAt0x20_partial_equals() {
        let lhs = CGRect(origin: CGPoint(x: 11.0, y: 17.0), size: CGSize(width: 5.0, height: 7.0))
        let rhs = CGRect(origin: CGPoint(x: 13.0, y: 19.0), size: CGSize(width: 5.0, height: 7.0))

        withUnsafePointer(to: lhs) { lhsPtr in
            withUnsafePointer(to: rhs) { rhsPtr in
                XCTAssertTrue(DGCompareValues(lhs: lhsPtr, rhs: rhsPtr,
                                              offset: MemoryLayout<CGRect>.offset(of: \.size)!,
                                              size: MemoryLayout<CGSize>.size,
                                              options: .equatable,
                                              type: CGRect.self))
            }
        }

    }


    func testViewGeometry_with_equatable_diff_size_only() {


        struct ViewOrigin: Equatable {
            internal var value: CGPoint
        }

        struct ViewSize: Equatable {

            internal var value: CGSize

            internal var _proposal: CGSize
        }

        class AnyLayoutEngineBox {

        }

        struct LayoutComputer: Equatable {
            internal var seed: Int


            internal var engine: AnyLayoutEngineBox

            internal static func == (lhs: LayoutComputer, rhs: LayoutComputer) -> Bool {
                lhs.seed == rhs.seed && lhs.engine === rhs.engine
            }
        }

        struct ViewDimensions: Equatable {

            var guideComputer: LayoutComputer
            var size: ViewSize
        }

        struct ViewGeometry: Equatable {
            let origin: ViewOrigin
            let value: Int = 0xfeedcafe
            let size: ViewDimensions
        }

        let layoutEngineBox = AnyLayoutEngineBox()
        let guideComputer = LayoutComputer(seed: 0, engine: layoutEngineBox)
        let origin = ViewOrigin(value: CGPoint(x: 11.0, y: 13.0))

        let lhsDimension = ViewDimensions(guideComputer: guideComputer,
                                          size: ViewSize(value: CGSize(width: 41.5, height: 71.5),
                                                         _proposal: CGSize(width: 1024, height: 1024)))

        let rhsDimension = ViewDimensions(guideComputer: guideComputer,
                                          size: ViewSize(value: CGSize(width: 41.5, height: 81.5),
                                                         _proposal: CGSize(width: 1024, height: 1024)))

        let lhsGeometry = ViewGeometry(origin: origin, size: lhsDimension)
        let rhsGeometry = ViewGeometry(origin: origin, size: rhsDimension)

        withUnsafePointer(to: lhsGeometry) { lhs in
            withUnsafePointer(to: rhsGeometry) { rhs in

                XCTAssertTrue(DGCompareValues(lhs: lhs, rhs: rhs,
                                              offset: MemoryLayout<ViewGeometry>.offset(of: \.value)! + MemoryLayout<Int8>.size,
                                              size: MemoryLayout<Int>.size - MemoryLayout<Int8>.size + MemoryLayout<LayoutComputer>.size,
                                              options: .inline,
                                              type: ViewGeometry.self))


                XCTAssertTrue(DGCompareValues(lhs: lhs, rhs: rhs,
                                              offset: MemoryLayout<ViewGeometry>.offset(of: \.origin.value.x)!,
                                              size: MemoryLayout<CGFloat>.size,
                                              options: .inline,
                                              type: ViewGeometry.self))


                XCTAssertTrue(DGCompareValues(lhs: lhs, rhs: rhs,
                                              offset: MemoryLayout<ViewGeometry>.offset(of: \.origin.value.y)!,
                                              size: MemoryLayout<CGFloat>.size,
                                              options: .inline,
                                              type: ViewGeometry.self))


                XCTAssertTrue(DGCompareValues(lhs: lhs, rhs: rhs,
                                              offset: MemoryLayout<ViewGeometry>.offset(of: \.size.size.value.width)!,
                                              size: MemoryLayout<CGFloat>.size,
                                              options: .inline,
                                              type: ViewGeometry.self))


                XCTAssertFalse(DGCompareValues(lhs: lhs, rhs: rhs,
                                               offset: MemoryLayout<ViewGeometry>.offset(of: \.size.size.value.height)!,
                                               size: MemoryLayout<CGFloat>.size,
                                               options: .inline,
                                               type: ViewGeometry.self))


                XCTAssertFalse(DGCompareValues(lhs: lhs, rhs: rhs,
                                               offset: MemoryLayout<ViewGeometry>.offset(of: \.origin)!,
                                               size: MemoryLayout<ViewGeometry>.size,
                                               options: .inline,
                                               type: ViewGeometry.self))


                XCTAssertTrue(DGCompareValues(lhs: lhs, rhs: rhs,
                                              offset: MemoryLayout<ViewGeometry>.offset(of: \.origin)!,
                                              size: MemoryLayout<ViewOrigin>.size + MemoryLayout<LayoutComputer>.size,
                                              options: .inline,
                                              type: ViewGeometry.self))


                XCTAssertTrue(DGCompareValues(lhs: lhs, rhs: rhs,
                                              offset: MemoryLayout<ViewGeometry>.offset(of: \.origin)!,
                                              size: MemoryLayout<ViewOrigin>.size,
                                              options: .inline,
                                              type: ViewGeometry.self))


                XCTAssertTrue(DGCompareValues(lhs: lhs, rhs: rhs,
                                              offset: MemoryLayout<ViewGeometry>.offset(of: \.size.guideComputer)!,
                                              size: MemoryLayout<LayoutComputer>.size,
                                              options: .inline,
                                              type: ViewGeometry.self))


                XCTAssertFalse(DGCompareValues(lhs: lhs, rhs: rhs,
                                               offset: MemoryLayout<ViewGeometry>.offset(of: \.size.size.value)!,
                                               size: MemoryLayout<CGSize>.size,
                                               options: .inline,
                                               type: ViewGeometry.self))


                XCTAssertTrue(DGCompareValues(lhs: lhs, rhs: rhs,
                                              offset: MemoryLayout<ViewGeometry>.offset(of: \.size.size._proposal)!,
                                              size: MemoryLayout<CGSize>.size,
                                              options: .inline,
                                              type: ViewGeometry.self))
            }
        }
    }

    func testViewGeometry_without_equatable_diff_size_only() {


        struct ViewOrigin {
            internal var value: CGPoint
        }

        struct ViewSize {

            internal var value: CGSize

            internal var _proposal: CGSize
        }

        class AnyLayoutEngineBox {

        }

        struct LayoutComputer: Equatable {
            internal var seed: Int


            internal var engine: AnyLayoutEngineBox

            internal static func == (lhs: LayoutComputer, rhs: LayoutComputer) -> Bool {
                lhs.seed == rhs.seed && lhs.engine === rhs.engine
            }
        }

        struct ViewDimensions {

            var guideComputer: LayoutComputer
            var size: ViewSize
        }

        struct ViewGeometry {
            let origin: ViewOrigin
            let value: Int = 0xfeedcafe
            let size: ViewDimensions
        }

        let layoutEngineBox = AnyLayoutEngineBox()
        let guideComputer = LayoutComputer(seed: 0, engine: layoutEngineBox)
        let origin = ViewOrigin(value: CGPoint(x: 11.0, y: 13.0))

        let lhsDimension = ViewDimensions(guideComputer: guideComputer,
                                          size: ViewSize(value: CGSize(width: 41.5, height: 71.5),
                                                         _proposal: CGSize(width: 1024, height: 1024)))

        let rhsDimension = ViewDimensions(guideComputer: guideComputer,
                                          size: ViewSize(value: CGSize(width: 41.5, height: 81.5),
                                                         _proposal: CGSize(width: 1024, height: 1024)))

        let lhsGeometry = ViewGeometry(origin: origin, size: lhsDimension)
        let rhsGeometry = ViewGeometry(origin: origin, size: rhsDimension)

        withUnsafePointer(to: lhsGeometry) { lhs in
            withUnsafePointer(to: rhsGeometry) { rhs in

                XCTAssertTrue(DGCompareValues(lhs: lhs, rhs: rhs,
                                              offset: MemoryLayout<ViewGeometry>.offset(of: \.value)! + MemoryLayout<Int8>.size,
                                              size: MemoryLayout<Int>.size - MemoryLayout<Int8>.size + MemoryLayout<LayoutComputer>.size,
                                              options: .inline,
                                              type: ViewGeometry.self))


                XCTAssertTrue(DGCompareValues(lhs: lhs, rhs: rhs,
                                              offset: MemoryLayout<ViewGeometry>.offset(of: \.origin.value.x)!,
                                              size: MemoryLayout<CGFloat>.size,
                                              options: .inline,
                                              type: ViewGeometry.self))


                XCTAssertTrue(DGCompareValues(lhs: lhs, rhs: rhs,
                                              offset: MemoryLayout<ViewGeometry>.offset(of: \.origin.value.y)!,
                                              size: MemoryLayout<CGFloat>.size,
                                              options: .inline,
                                              type: ViewGeometry.self))


                XCTAssertTrue(DGCompareValues(lhs: lhs, rhs: rhs,
                                              offset: MemoryLayout<ViewGeometry>.offset(of: \.size.size.value.width)!,
                                              size: MemoryLayout<CGFloat>.size,
                                              options: .inline,
                                              type: ViewGeometry.self))


                XCTAssertFalse(DGCompareValues(lhs: lhs, rhs: rhs,
                                               offset: MemoryLayout<ViewGeometry>.offset(of: \.size.size.value.height)!,
                                               size: MemoryLayout<CGFloat>.size,
                                               options: .inline,
                                               type: ViewGeometry.self))


                XCTAssertFalse(DGCompareValues(lhs: lhs, rhs: rhs,
                                               offset: MemoryLayout<ViewGeometry>.offset(of: \.origin)!,
                                               size: MemoryLayout<ViewGeometry>.size,
                                               options: .inline,
                                               type: ViewGeometry.self))


                XCTAssertTrue(DGCompareValues(lhs: lhs, rhs: rhs,
                                              offset: MemoryLayout<ViewGeometry>.offset(of: \.origin)!,
                                              size: MemoryLayout<ViewOrigin>.size + MemoryLayout<LayoutComputer>.size,
                                              options: .inline,
                                              type: ViewGeometry.self))


                XCTAssertTrue(DGCompareValues(lhs: lhs, rhs: rhs,
                                              offset: MemoryLayout<ViewGeometry>.offset(of: \.origin)!,
                                              size: MemoryLayout<ViewOrigin>.size,
                                              options: .inline,
                                              type: ViewGeometry.self))


                XCTAssertTrue(DGCompareValues(lhs: lhs, rhs: rhs,
                                              offset: MemoryLayout<ViewGeometry>.offset(of: \.size.guideComputer)!,
                                              size: MemoryLayout<LayoutComputer>.size,
                                              options: .inline,
                                              type: ViewGeometry.self))


                XCTAssertFalse(DGCompareValues(lhs: lhs, rhs: rhs,
                                               offset: MemoryLayout<ViewGeometry>.offset(of: \.size.size.value)!,
                                               size: MemoryLayout<CGSize>.size,
                                               options: .inline,
                                               type: ViewGeometry.self))


                XCTAssertTrue(DGCompareValues(lhs: lhs, rhs: rhs,
                                              offset: MemoryLayout<ViewGeometry>.offset(of: \.size.size._proposal)!,
                                              size: MemoryLayout<CGSize>.size,
                                              options: .inline,
                                              type: ViewGeometry.self))
            }
        }
    }

}
