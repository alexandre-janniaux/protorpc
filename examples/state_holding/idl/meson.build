state_idl_sources = sidlcc.process('database.sidl')
state_idl_lib = static_library('stateidl', state_idl_sources,
  dependencies: [protorpc_dep]
)

state_idl_dep = declare_dependency(
  link_with: state_idl_lib,
  include_directories: state_idl_lib.private_dir_include()
)
