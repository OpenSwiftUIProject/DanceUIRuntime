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
import SwiftUI

@available(iOS 13.0, *)
final class DanceUILayoutDescriptorTests: XCTestCase {

    override func setUpWithError() throws {

    }

    override func tearDownWithError() throws {

    }

    let types: [Any.Type] = [
        Int.self, String.self, [String].self, [String: Int].self,
        Image.self, Text.self, Color.self, AnyView.self,
        Picker<Text, String, Text>.self,
        _ConditionalContent<Color, Picker<Text, String, Text>>.self,
        (Image, Text, Color, _ConditionalContent<AnyView, AnyView>).self,
        Button<Text>.self, Button<Text>.Body.self,
        State<String>.self, Binding<String>.self
    ]

    func test_makeLayouts_performance() throws {

        self.measure {

            for type in types {
                TypeID(type).fetch()
            }
        }
    }

}
