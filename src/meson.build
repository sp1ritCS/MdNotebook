src = [
  'mdnotebookbuffer.c',
  'mdnotebookbufwidget.c',
  'mdnotebookview.c',
  'mdnotebookzoomview.c',
  [
    'bufitem/mdnotebookbufitem.c',
    'bufitem/mdnotebookbufitemdynblock.c',
    'bufitem/mdnotebookbufitemheading.c',
    'bufitem/mdnotebookbufitemtext.c'
  ]
]

headers = [
  'mdnotebook.h',
  'mdnotebookbuffer.h',
  'mdnotebookbufferextra.h',
  'mdnotebookbufwidget.h',
  'mdnotebookview.h',
  'mdnotebookzoomview.h',
  [
    'bufitem/mdnotebookbufitem.h',
    'bufitem/mdnotebookbufitemdynblock.h',
    'bufitem/mdnotebookbufitemheading.h',
    'bufitem/mdnotebookbufitemtext.h'
  ]
]

mdnotebook_enums = gnome.mkenums('mdnotebook_enums',
  sources: headers,
  h_template: 'mdnotebook_enums.h.in',
  c_template: 'mdnotebook_enums.c.in',
  symbol_prefix: 'mdnotebook',
  install_header: true,
)

mdnotebook_lib = library('mdnotebook', src + mdnotebook_enums,
  dependencies: [
    dependency('glib-2.0'),
    dependency('gtk4')
  ],
  install: true
)

mdnotebook_dep = declare_dependency(
  link_with: mdnotebook_lib,
  include_directories: include_directories('.')
)

install_headers(headers, subdir: 'mdnotebook')

pkgconfig = import('pkgconfig')
pkgconfig.generate(
  name: 'GtkMdNotebook',
  filebase: 'mdnotebook',
  description: 'Lorem ipsum dolor sit amet',
  subdirs: 'mdnotebook',
  libraries: mdnotebook_lib,
  version: meson.project_version()
)

gnome.generate_gir(mdnotebook_lib,
  sources: src + headers + mdnotebook_enums,
  namespace: 'MdNotebook',
  nsversion: '1',
  symbol_prefix: 'mdnotebook',
  dependencies: [
    dependency('glib-2.0'),
    dependency('gtk4')
  ],
  includes: ['GLib-2.0', 'GObject-2.0', 'Gtk-4.0'],
  header: 'mdnotebook.h',
  install: true
)
