protoipc_sources = []

if build_machine.system() == 'linux'
  protoipc_sources += [
    'src/linux_port.cpp',
    'src/linux_router.cpp'
  ]
else
  error('Unsupported os: @0@'.format(build_machine.system()))
endif

protoipc_headers = include_directories('include')

protoipc_library = library('protoipc', protoipc_sources,
  include_directories: protoipc_headers,
  install: true,
)

protoipc_dep = declare_dependency(link_with: protoipc_library,
  include_directories: protoipc_headers
)

# Sometimes we cannot rely on both libprotoipc and libprotorpc to be present.
# As such we need to expose a static protoipc library so that protorpc can
# link to it.
protoipc_static_library = static_library('protoipc-static', protoipc_sources,
  include_directories: protoipc_headers
)

protoipc_headers_dep = declare_dependency(
  include_directories: protoipc_headers
)

protoipc_install_headers = [
  'include/protoipc/port.hh',
  'include/protoipc/message.hh',
  'include/protoipc/router.hh'
]

pkg = import('pkgconfig')
pkg.generate(protoipc_library)

install_headers(protoipc_install_headers, subdir: 'protoipc')

if get_option('protoipc_tests')
  gtest = subproject('gtest')
  gtest_dep = gtest.get_variable('gtest_dep')

  protoipc_tests = executable('protoipc_tests',
    'tests/ipc_tests.cpp',
    dependencies: [gtest_dep, protoipc_dep]
  )

  test('protoipc tests', protoipc_tests)
endif
