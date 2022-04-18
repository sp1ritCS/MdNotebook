src = [
  'gtkmdnotebookbuffer.c',
  'gtkmdnotebookbufwidget.c',
  'gtkmdnotebookview.c',
  'gtkmdnotebookzoomview.c',
  [
    'bufitem/mdnotebookbufitem.c',
    'bufitem/mdnotebookbufitemheading.c',
    'bufitem/mdnotebookbufitemtext.c'
  ]
]

headers = [
  'gtkmdnotebook.h',
  'gtkmdnotebookbuffer.h',
  'gtkmdnotebookbufferextra.h',
  'gtkmdnotebookbufwidget.h',
  'gtkmdnotebookview.h',
  'gtkmdnotebookzoomview.h',
  [
    'bufitem/mdnotebookbufitem.h',
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

gtkmdnotebook_lib = library('gtkmdnotebook', src + mdnotebook_enums,
  dependencies: [
    dependency('glib-2.0'),
    dependency('gtk4')
  ],
  install: true
)

gtkmdnotebook_dep = declare_dependency(
  link_with: gtkmdnotebook_lib,
  include_directories: include_directories('.')
)

install_headers(headers, subdir: 'gtkmdnotebook')

pkgconfig = import('pkgconfig')
pkgconfig.generate(
  name: 'GtkMdNotebook',
  filebase: 'gtkmdnotebook',
  description: 'Lorem ipsum dolor sit amet',
  subdirs: 'gtkmdnotebook',
  libraries: gtkmdnotebook_lib,
  version: meson.project_version()
)

gnome.generate_gir(gtkmdnotebook_lib,
  sources: src + headers + mdnotebook_enums,
  namespace: 'MdNotebook',
  nsversion: '1',
  symbol_prefix: 'mdnotebook',
  dependencies: [
    dependency('glib-2.0'),
    dependency('gtk4')
  ],
  includes: ['GLib-2.0', 'GObject-2.0', 'Gtk-4.0'],
  header: 'gtkmdnotebook.h',
  install: true
)
