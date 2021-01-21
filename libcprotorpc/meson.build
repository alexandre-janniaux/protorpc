cprotorpc_headers = include_directories('include')
cprotorpc_sources = [
  'src/serializer.c',
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
  'include/cprotorpc/structures.h'
]

pkg = import('pkgconfig')
pkg.generate(cprotorpc_library)

install_headers(cprotorpc_install_headers, subdir: 'cprotorpc')
