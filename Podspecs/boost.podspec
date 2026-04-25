Pod::Spec.new do |s|
  s.name             = 'boost'
  s.version          = '1.88.0'
  s.summary          = 'Boost C++ Libraries (Header-Only)'

  s.description      = <<-DESC
    Boost provides free peer-reviewed portable C++ source libraries.
    This pod contains header-only libraries for DanceUIGraph.
  DESC

  s.homepage         = 'https://www.boost.org'
  s.license          = { :type => 'Boost Software License', :file => 'boost/LICENSE_1_0.txt' }
  s.author           = { 'Boost' => 'boost@boost.org' }
  s.source           = { :path => '.' }

  s.ios.deployment_target = '13.0'
  s.osx.deployment_target = '10.14'

  s.requires_arc = false
  s.module_name = 'boost'
  s.header_dir = 'boost'
  s.preserve_paths = ['boost']
end
