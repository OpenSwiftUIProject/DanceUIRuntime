swift_version = "6.1.2"
swift_arch_type = `uname -m`.strip
swift_build_platform = "macosx-#{swift_arch_type}"

Pod::Spec.new do |s|
  s.name             = 'swift'
  s.version          = swift_version
  s.summary          = 'Swift Programming Language Headers'

  s.description      = <<-DESC
    Swift compiler and runtime headers for DanceUIGraph.
    Provides header-only access to Swift, LLVM, Clang, and cmark.
  DESC

  s.homepage         = 'https://swift.org'
  s.license          = { :type => 'Apache License 2.0', :file => 'Source/swift/LICENSE.txt' }
  s.author           = { 'Apple Inc.' => 'swift@apple.com' }
  s.source           = { :path => '.' }

  s.ios.deployment_target = '13.0'
  s.osx.deployment_target = '10.14'

  s.xcconfig = {
    'HEADER_SEARCH_PATHS' => '$(PROJECT_ROOT)/../DanceUIDependencies/swift/Source/swift/include/',
    'SWIFT_BASE_LOCATION' => '$(PROJECT_ROOT)/../DanceUIDependencies/swift/Source/',
    'SWIFT_BUILD_LOCATION_NAME' => 'build',
    'SWIFT_BUILD_TOOL_NAME' => 'Ninja',
    'SWIFT_BUILD_MODE_NAME' => 'ReleaseAssert',
    'SWIFT_BUILD_PLATFORM_NAME' => swift_build_platform,
    'SWIFT_BUILD_LLVMLIB_MODE' => 'ReleaseAssert',
    'SWIFT_BUILD_SWIFTLIB_MODE' => 'ReleaseAssert'
  }

  s.library = 'c++'
  s.requires_arc = false

  s.preserve_paths = [
    "Source/swift",
    "Source/build",
    "Source/llvm-project",
    "Source/cmark",
    "Source/swift-driver"
  ]

  # No prepare_command - init.sh handles setup
end
