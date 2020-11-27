state_sources = [
  'main.cpp'
]

subdir('idl')

thread_dep = dependency('threads')

state = executable('state_db', state_sources,
  dependencies: [protorpc_dep, state_idl_dep, thread_dep]
)
