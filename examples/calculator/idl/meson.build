calculator_idl_sources = sidlcc.process('calculator.sidl')
calculator_idl_lib = static_library('calculatoridl', calculator_idl_sources,
  dependencies: [protorpc_dep])

calculator_idl_dep = declare_dependency(
  link_with: calculator_idl_lib,
  include_directories: calculator_idl_lib.private_dir_include()
)
