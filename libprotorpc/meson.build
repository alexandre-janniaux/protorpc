protorpc_sources = []
protorpc_headers = include_directories('include')

protorpc_sources += [
  'src/channel.cpp',
  'src/rpcobject.cpp',
  'src/exrpcobject.cpp',
  'src/exchannel.cpp'
]

if build_machine.system() == 'linux'
  protorpc_sources += [ 'src/broker.cpp' ]
else
  error('Unsupported os: @0@'.format(build_machine.system()))
endif

protorpc_library = library('protorpc', protorpc_sources,
  include_directories: protorpc_headers,
  dependencies: [protoipc_dep, fmt_dep],
  install: true
)

protorpc_dep = declare_dependency(link_with: protorpc_library,
  include_directories: protorpc_headers,
  dependencies: [protoipc_dep]
)

if get_option('protorpc_tests')
  gtest = subproject('gtest')
  gtest_dep = gtest.get_variable('gtest_dep')

  protorpc_tests = executable('protorpc_simple_tests',
    'tests/rpc_simple_tests.cpp',
    dependencies: [gtest_dep, protorpc_dep, fmt_dep]
  )

  test('protorpc simple tests', protorpc_tests)
endif
