src = [
  'gtkmdnotebookview.c',
  'gtkmdnotebookbuffer.c',
  'gtkmdnotebookbufwidget.c',
  'gtkmdnotebookzoomview.c'
]

gtkmdnotebook_lib = library('gtkmdnotebook', src,
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

install_headers(['gtkmdnotebook.h', 'gtkmdnotebookview.h', 'gtkmdnotebookbuffer.h', 'gtkmdnotebookbufwidget.h'], subdir: 'gtkmdnotebook')

pkgconfig = import('pkgconfig')
pkgconfig.generate(
  name: 'GtkMdNotebook',
  filebase: 'gtkmdnotebook',
  description: 'Lorem ipsum dolor sit amet',
  subdirs: 'gtkmdnotebook',
  libraries: gtkmdnotebook_lib,
  version: meson.project_version()
)
