project(
  'sortbench', 'cpp',
  version: '0.1.0',
  license: 'MIT',
  license_files: 'LICENSE',
  meson_version: '>=1.1.0',
  default_options: [
    'cpp_std=c++17',
    'warning_level=3',
    'buildtype=debugoptimized',
    'b_ndebug=if-release',
  ],
)

cpp = meson.get_compiler('cpp')

# Cross-platform build flags. Everything else is standard C++17 so the same
# sources build with MSVC, GCC and Clang.
if cpp.get_id() == 'msvc'
  add_project_arguments('/utf-8', '/permissive-', '/Zc:__cplusplus', language: 'cpp')
else
  add_project_arguments('-Wshadow', language: 'cpp')
endif

subdir('src')

if get_option('build_tests')
  subdir('tests')
endif
