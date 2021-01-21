cprotorpc_headers = include_directories('include')
cprotorpc_sources = [
  'src/serializer.c',
  'src/unserializer.c',
  'src/structures.c'
]

cprotorpc_library = library('cprotorpc', cprotorpc_sources,
  include_directories: cprotorpc_headers,
  install: true
)

cprotorpc_dep = declare_dependency(link_with: cprotorpc_library,
  include_directories: cprotorpc_headers
)

cprotorpc_install_headers = [
  'include/cprotorpc/serializer.h',
  'include/cprotorpc/unserializer.h',
  'include/cprotorpc/structures.h'
]

pkg = import('pkgconfig')
pkg.generate(cprotorpc_library)

install_headers(cprotorpc_install_headers, subdir: 'cprotorpc')

if get_option('cprotorpc_tests')
  gtest = subproject('gtest')
  gtest_dep = gtest.get_variable('gtest_dep')

  protorpc_tests = executable('cprotorpc_serialization_tests',
    'tests/serialization_tests.cpp',
    dependencies: [gtest_dep, cprotorpc_dep]
  )

  test('protorpc simple tests', protorpc_tests)
endif
