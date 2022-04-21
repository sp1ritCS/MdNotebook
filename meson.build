project('MdNotebook', 'c',
  version : '0.1',
  default_options : ['warning_level=3', 'c_std=gnu2x', 'cpp_std=c++17',
    'microtex:QT=false', 'microtex:EXAMPLE_QT=false', 'microtex:EXAMPLE_GTK=false', 'microtex:GDI=disabled', 'microtex:EXAMPLE_WIN32=false'
  ]
)

prefix = get_option('prefix')
include_dir = join_paths(prefix, get_option('includedir'))

gnome = import('gnome')

subdir('src')
subdir('demo')
