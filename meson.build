project('protorpc', 'cpp',
  default_options: ['cpp_std=c++17'],
  version: '0.0.1')

fmt = subproject('fmt', default_options: ['default_library=static'])
fmt_dep = fmt.get_variable('fmt_dep')
threads_dep = dependency('threads')

subdir('libprotoipc')
subdir('libprotorpc')
subdir('sidl')

if get_option('build_examples')
  subdir('examples')
endif
