calculator_sources = [
  'main.cpp'
]

subdir('idl')

thread_dep = dependency('threads')

calculator = executable('calculator', calculator_sources,
  dependencies: [protorpc_dep, calculator_idl_dep, thread_dep]
)
