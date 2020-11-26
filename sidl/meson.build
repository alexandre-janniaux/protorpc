sidlcc = generator(find_program('sidlcc.py'),
  output: ['@PLAINNAME@.cpp', '@PLAINNAME@.hh'],
  arguments: ['-o', '@BUILD_DIR@', '@INPUT@']
)

# Install sidlcc.py + sidl package into $(prefix)/bin
# It is a hack as it installs the python package inside /bin as well.
if get_option('sidl_vlc_contrib')
  install_data('sidlcc.py', install_dir: 'bin')
  install_subdir('sidl', install_dir: 'bin')
endif
