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
import DanceUIRuntime

private struct EscapedClosureContainedValue {

    let escapedClosure: () -> Void

}


func testRulePartialUpdateRelayWithInlineComparisonModeAndEscapedClosureContainedValueHeapCapturedVariable() {
    typealias Value = EscapedClosureContainedValue
    class Foo {
    }

    let foo = Foo()
    let initialValue = Value {
        let _ = foo
    }
    precondition(true == DGCompareValues(lhs: initialValue, rhs: initialValue, options: .inline))


}

func test() {


    testRulePartialUpdateRelayWithInlineComparisonModeAndEscapedClosureContainedValueHeapCapturedVariable()

    var value1: CGRect = .init(origin: CGPoint(x: 0, y: 0), size: CGSize(width: 200, height: 200))
    var value2: CGRect = .init(origin: CGPoint(x: 1, y: 2), size: CGSize(width: 200, height: 200))

    let result = DGCompareValues(lhs: UnsafeRawPointer(&value1),
                                 rhs: UnsafeRawPointer(&value2),
                                 offset: MemoryLayout.offset(of: \CGRect.size)!,
                                 size: MemoryLayout<CGSize>.size,
                                 type: CGRect.self)
    precondition(result == true)
    print(applyFields)

}
