conf_data = configuration_data()

src = [
  'mdnotebookbuffer.c',
  'mdnotebookbufwidget.c',
  'mdnotebookdrawing.c',
  'mdnotebooktoolbar.c',
  'mdnotebookview.c',
  'mdnotebookzoomview.c',
  [
    'booktool/mdnotebookbooktool.c',
    'booktool/mdnotebookbooktoolpen.c',
    'booktool/mdnotebookbooktooltext.c'
  ],
  [
    'bufitem/mdnotebookbufitem.c',
    'bufitem/mdnotebookbufitemcodeblock.c',
    'bufitem/mdnotebookbufitemdynblock.c',
    'bufitem/mdnotebookbufitemheading.c',
    'bufitem/mdnotebookbufitemtext.c',

    'bufitem/mdnotebookproxbufitem.c',
    'bufitem/mdnotebookbufitemcheckmark.c',
  ]
]

headers = [
  'mdnotebook.h',
  'mdnotebookbuffer.h',
  'mdnotebookbufferextra.h',
  'mdnotebookbufwidget.h',
  'mdnotebookdrawing.h',
  'mdnotebooktoolbar.h',
  'mdnotebookview.h',
  'mdnotebookviewextra.h',
  'mdnotebookzoomview.h',
  [
    'booktool/mdnotebookbooktool.h',
    'booktool/mdnotebookbooktoolpen.h',
    'booktool/mdnotebookbooktooltext.h'
  ],
  [
    'bufitem/mdnotebookbufitem.h',
    'bufitem/mdnotebookbufitemcodeblock.h',
    'bufitem/mdnotebookbufitemdynblock.h',
    'bufitem/mdnotebookbufitemheading.h',
    'bufitem/mdnotebookbufitemtext.h',

    'bufitem/mdnotebookproxbufitem.h',
    'bufitem/mdnotebookbufitemcheckmark.h',
  ]
]


microtex = dependency('microtex', version: '>=1.0.0', fallback: ['microtex', 'microtex_dep'], required: get_option('latex'))
microtex_cairo = dependency('microtex-cairo', version: '>=1.0.0', fallback: ['microtex', 'microtex_cairo_dep'], required: get_option('latex'))
if microtex.found() and microtex_cairo.found()
  if add_languages('cpp', required: get_option('latex'))
    src += [
      'bufitem/latex/mdnotebookbufitemlatex.c',
      'bufitem/latex/mdnotebooklatexequation.cpp',
      'bufitem/latex2/mdnotebookbufitemlatextwo.c',
    ]
    headers += [
      'bufitem/latex/mdnotebookbufitemlatex.h',
      'bufitem/latex/mdnotebooklatexequation.h',
      'bufitem/latex2/mdnotebookbufitemlatextwo.h',
    ]

    conf_data.set('MDNOTEBOOK_HAVE_LATEX', true)
  endif
endif

headers += configure_file(input: 'mdnotebookconfig.h.in',
  output: 'mdnotebookconfig.h',
  configuration: conf_data
)

mdnotebook_enums = gnome.mkenums('mdnotebook_enums',
  sources: headers,
  h_template: 'mdnotebook_enums.h.in',
  c_template: 'mdnotebook_enums.c.in',
  symbol_prefix: 'mdnotebook',
  install_header: true,
  install_dir: join_paths(include_dir, 'mdnotebook')
)

mdnotebook_lib = library('mdnotebook', src + mdnotebook_enums,
  dependencies: [
    dependency('glib-2.0'),
    dependency('gtk4'),
    [microtex, microtex_cairo]
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
