protorpc_sources = []
protorpc_headers = include_directories('include')

protorpc_sources += [
  'src/channel.cpp',
]

# Link whole is needed to embed all code from libprotoipc statically (even code
# unused by the rpc such as ipc::Router).
protorpc_library = library('protorpc', protorpc_sources,
  include_directories: protorpc_headers,
  dependencies: [protoipc_headers_dep],
  link_whole: protoipc_static_library,
  install: true
)

protorpc_dep = declare_dependency(link_with: protorpc_library,
  include_directories: protorpc_headers,
  dependencies: [protoipc_headers_dep]
)

protorpc_install_headers = [
  'include/protorpc/channel.hh',
  'include/protorpc/message.hh',
  'include/protorpc/rpcobject.hh',
  'include/protorpc/serializer.hh',
  'include/protorpc/unserializer.hh'
]

pkg = import('pkgconfig')
pkg.generate(protorpc_library)

install_headers(protorpc_install_headers, subdir: 'protorpc')

if get_option('protorpc_tests')
  gtest = subproject('gtest')
  gtest_dep = gtest.get_variable('gtest_dep')

  protorpc_tests = executable('protorpc_simple_tests',
    'tests/rpc_simple_tests.cpp',
    dependencies: [gtest_dep, protorpc_dep, fmt_dep]
  )

  test('protorpc simple tests', protorpc_tests)
endif
