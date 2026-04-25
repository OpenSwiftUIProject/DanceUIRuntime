source = 'https://github.com/bytedance/DanceUIRuntime.git'

# "danceui_use_dynamic" is a global environment value that can be passed from
# cocoapods-danceui or other tooling to toggle dynamic frameworks.
is_dynamic = defined?(danceui_use_dynamic?) ? danceui_use_dynamic? : false

build_library_for_distribution = "YES"
if defined?(enable_distribution?)
  build_library_for_distribution = enable_distribution? ? "YES" : build_library_for_distribution
end
build_library_for_distribution = "YES"

swift_version = '6.1.2'

swift_source_dir="$(PODS_TARGET_SRCROOT)/../DanceUIDependencies/swift/Source"
swift_build_tool="Ninja"
swift_build_mode="ReleaseAssert"
swift_product_name="swift"
swift_arch_type=`uname -m`
swift_build_platform="macosx-#{swift_arch_type}"
swift_build_dir="#{swift_source_dir}/build/#{swift_build_tool}-#{swift_build_mode}/#{swift_product_name}-#{swift_build_platform}/"

#puts "[DanceUIRuntime] Configuring DanceUIRuntime #{swift_build_dir}"

class Pod::Spec
  # Version Manager defines: handle DanceUIGraph_POD_VERSION and DanceUIGraph_COMMIT_ID
  def danceui_runtime_version_manager_gcc_preprocessor_definitions
      version_raw = "#{name}_POD_VERSION=@\\\"#{version}\\\""

      commit_raw = ""
      begin
          commit_id = `git rev-parse --short=8 HEAD`.strip!
          commit_raw = "#{name}_COMMIT_ID=@\\\"#{commit_id}\\\""
      rescue
          commit_raw = ""
      end
      "#{version_raw} #{commit_raw}".strip
  end
end

Pod::Spec.new do |s|
  s.name                                           = 'DanceUIRuntime'
  s.version                                        = '0.1.0'
  s.summary                                        = 'DanceUI Core Engine Support.'

  # This description is used to generate tags and improve search results.
  #         * Think: What does it do? Why did you write it? What is the focus?
  #         * Try to keep it short, snappy and to the point.
  #         * Write the description between the DESC delimiters below.
  #         * Finally, don't worry about the indent, CocoaPods strips it!

  s.description                        = <<-DESC
  DanceUI Core Engine Support.

  DESC

  s.homepage            = 'https://github.com/bytedance/DanceUIRuntime'
  s.license             = { :type => 'Apache', :file => 'LICENSE' }
  s.author              = { 'bytedance' => 'bytedance@bytedance.com' }
  s.source              = { :git => source, :branch => 'release_'+s.version.to_s }
  s.swift_versions      = ['5.0', '5.1', '5.2', '5.3', '5.3.2', '5.5.1', '5.6.0', '5.9.0', '6.0']

  s.pod_target_xcconfig =        {
    'USER_HEADER_SEARCH_PATHS' => '$(inherited) $(PODS_TARGET_SRCROOT)/../DanceUIDependencies/boost ' +
    '"$(PODS_TARGET_SRCROOT)/../DanceUIDependencies/" ',
    'HEADER_SEARCH_PATHS' => '$(inherited) $(PODS_TARGET_SRCROOT)/DanceUIRuntime/Sources/DanceUIRuntime/include/ $(PODS_TARGET_SRCROOT)/DanceUIGraph/Sources/DanceUIGraph/umbrella/  $(PODS_TARGET_SRCROOT)/DanceUIGraph/Sources/DanceUIGraph/src/ $(PODS_TARGET_SRCROOT)/../DanceUIDependencies/boost/ $(PODS_ROOT)/DanceUIRuntime/Sources/DanceUIRuntime/include/ $(SWIFT_BASE_LOCATION)$(SWIFT_BUILD_LOCATION_NAME)/$(SWIFT_BUILD_TOOL_NAME)-$(SWIFT_BUILD_MODE_NAME)/swift-$(SWIFT_BUILD_PLATFORM_NAME)/include/ $(SWIFT_BASE_LOCATION)swift/include/ $(SWIFT_BASE_LOCATION)swift/stdlib/public/SwiftShims/ $(SWIFT_BASE_LOCATION)swift/include/ $(SWIFT_BASE_LOCATION)swift/stdlib $(SWIFT_BASE_LOCATION)swift/include/ $(SWIFT_BASE_LOCATION)llvm-project/llvm/include/ $(SWIFT_BASE_LOCATION)$(SWIFT_BUILD_LOCATION_NAME)/$(SWIFT_BUILD_TOOL_NAME)-$(SWIFT_BUILD_MODE_NAME)/llvm-$(SWIFT_BUILD_PLATFORM_NAME)/include/ $(SWIFT_BASE_LOCATION)llvm-project/clang/include/ $(SWIFT_BASE_LOCATION)$(SWIFT_BUILD_LOCATION_NAME)/$(SWIFT_BUILD_TOOL_NAME)-$(SWIFT_BUILD_MODE_NAME)/llvm-$(SWIFT_BUILD_PLATFORM_NAME)/tools/clang/include $(SWIFT_BASE_LOCATION)cmark/src/ $(SWIFT_BASE_LOCATION)$(SWIFT_BUILD_LOCATION_NAME)/$(SWIFT_BUILD_TOOL_NAME)-$(SWIFT_BUILD_MODE_NAME)/cmark-$(SWIFT_BUILD_PLATFORM_NAME)/src/',

    'CLANG_CXX_LANGUAGE_STANDARD' => 'c++17',
    'LIBRARY_SEARCH_PATHS' => '$(SDKROOT)/usr/lib/swift',
    'ENABLE_NS_ASSERTIONS' => 'YES',
    'OTHER_CPLUSPLUSFLAGS' => '$(OTHER_CFLAGS) -fno-aligned-allocation',
    "BUILD_LIBRARY_FOR_DISTRIBUTION" => build_library_for_distribution,


    'SWIFT_BASE_LOCATION' => "#{swift_source_dir}/",
    'SWIFT_BUILD_LOCATION_NAME' => 'build',
    'SWIFT_BUILD_TOOL_NAME' => "#{swift_build_tool}",
    'SWIFT_BUILD_MODE_NAME' => "#{swift_build_mode}",
    'SWIFT_BUILD_PLATFORM_NAME' => "#{swift_build_platform}",
    'SWIFT_BUILD_LLVMLIB_MODE' => "#{swift_build_mode}",
    'SWIFT_BUILD_SWIFTLIB_MODE' => "#{swift_build_mode}",

    # VersionManager
    "GCC_PREPROCESSOR_DEFINITIONS" => "$(inherited) #{s.danceui_runtime_version_manager_gcc_preprocessor_definitions}"
  }

  s.ios.deployment_target = '13.0'
  s.osx.deployment_target = '10.15'
  #s.module_map = 'DanceUIRuntime.modulemap'

  s.public_header_files = 'DanceUIRuntime/Sources/DanceUIRuntime/include/**/*.h', 'DanceUIRuntime/Sources/DanceUIRuntime/include/*.h'
  s.private_header_files = 'DanceUIRuntime/Sources/DanceUIRuntime/src/**/*.{h,hpp}', 'DanceUIRuntime/Sources/DanceUIRuntime/include/**/*.{hpp}'
  s.source_files = 'DanceUIRuntime/Sources/DanceUIRuntime/include/**/*', 'DanceUIRuntime/Sources/DanceUIRuntime/src/**/*'
  s.dependency 'boost', '~> 1.88.0'
  s.dependency 'swift', "~> #{swift_version}"

  s.requires_arc = false

  s.library = 'c++'
  s.static_framework = !is_dynamic

  s.test_spec 'UnitTests' do |test_spec|
    test_spec.test_type = 'unit'
    test_spec.pod_target_xcconfig = {
        'USER_HEADER_SEARCH_PATHS' => '$(PODS_TARGET_SRCROOT)/../DanceUIDependencies/boost ' +
        '"$(PODS_TARGET_SRCROOT)/../DanceUIDependencies/" ',
        'HEADER_SEARCH_PATHS' => '$(PODS_TARGET_SRCROOT)/DanceUIRuntime/Sources/DanceUIRuntime/src/ $(PODS_TARGET_SRCROOT)/../DanceUIDependencies/boost/ $(PODS_ROOT)/DanceUIRuntime/Sources/DanceUIRuntime/include/ $(SWIFT_BASE_LOCATION)$(SWIFT_BUILD_LOCATION_NAME)/$(SWIFT_BUILD_TOOL_NAME)-$(SWIFT_BUILD_MODE_NAME)/swift-$(SWIFT_BUILD_PLATFORM_NAME)/include/ $(SWIFT_BASE_LOCATION)swift/include/ $(SWIFT_BASE_LOCATION)swift/stdlib/public/SwiftShims/ $(SWIFT_BASE_LOCATION)swift/include/ $(SWIFT_BASE_LOCATION)swift/stdlib $(SWIFT_BASE_LOCATION)swift/include/ $(SWIFT_BASE_LOCATION)llvm-project/llvm/include/ $(SWIFT_BASE_LOCATION)$(SWIFT_BUILD_LOCATION_NAME)/$(SWIFT_BUILD_TOOL_NAME)-$(SWIFT_BUILD_MODE_NAME)/llvm-$(SWIFT_BUILD_PLATFORM_NAME)/include/ $(SWIFT_BASE_LOCATION)llvm-project/clang/include/ $(SWIFT_BASE_LOCATION)$(SWIFT_BUILD_LOCATION_NAME)/$(SWIFT_BUILD_TOOL_NAME)-$(SWIFT_BUILD_MODE_NAME)/llvm-$(SWIFT_BUILD_PLATFORM_NAME)/tools/clang/include $(SWIFT_BASE_LOCATION)cmark/src/ $(SWIFT_BASE_LOCATION)$(SWIFT_BUILD_LOCATION_NAME)/$(SWIFT_BUILD_TOOL_NAME)-$(SWIFT_BUILD_MODE_NAME)/cmark-$(SWIFT_BUILD_PLATFORM_NAME)/src/',
        'CLANG_CXX_LANGUAGE_STANDARD' => 'c++17',
        'ENABLE_NS_ASSERTIONS' => 'YES',
        'OTHER_CPLUSPLUSFLAGS' => '$(OTHER_CFLAGS) -fno-aligned-allocation',
        'LIBRARY_SEARCH_PATHS' => '$(SDKROOT)/usr/lib/swift',
        "ARCHS" => "$(ARCHS_STANDARD_64_BIT)",
        "EXCLUDED_ARCHS[sdk=iphonesimulator*]" => "i386",
        "GCC_PREPROCESSOR_MACROS" => '$(inherited) __DANCE_UI_RUNTIME_ENABLE_TESTING__=1',


        'SWIFT_BASE_LOCATION' => "#{swift_source_dir}/",
        'SWIFT_BUILD_LOCATION_NAME' => 'build',
        'SWIFT_BUILD_TOOL_NAME' => "#{swift_build_tool}",
        'SWIFT_BUILD_MODE_NAME' => "#{swift_build_mode}",
        'SWIFT_BUILD_PLATFORM_NAME' => "#{swift_build_platform}",
        'SWIFT_BUILD_LLVMLIB_MODE' => "#{swift_build_mode}",
        'SWIFT_BUILD_SWIFTLIB_MODE' => "#{swift_build_mode}",
    }
    test_spec.requires_app_host = true
    test_spec.source_files = 'DanceUIRuntime/Tests/**/*.{h,m,hpp,cpp,mm,c,cc,swift}'
    test_spec.public_header_files = 'DanceUIRuntime/Sources/DanceUIRuntime/include/**/*.{h,hpp}'
    test_spec.private_header_files = 'DanceUIRuntime/Sources/DanceUIRuntime/src/**/*.{h,hpp}'
  end
end
