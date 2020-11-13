protoipc_sources = []

if build_machine.system() == 'linux'
  protoipc_sources += ['src/linux_port.cpp']
else
  error('Unsupported os: @0@'.format(build_machine.system()))
endif

protoipc_headers = include_directories('include')

protoipc_library = library('protoipc', protoipc_sources,
  include_directories: protoipc_headers,
  install: true
)

protoipc_dep = declare_dependency(link_with: protoipc_library,
  include_directories: protoipc_headers
)

protoipc_install_headers = [
  'include/protoipc/port.hh'
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
