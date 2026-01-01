glib_mkenums = find_program('glib-mkenums')

version_parts = meson.project_version().split('.')
micro_version = 0
if version_parts.length() > 2
  micro_version = version_parts[2].to_int()
endif
features_conf = configuration_data()
features_conf.set('LIBRSVG_MAJOR_VERSION', version_parts[0].to_int())
features_conf.set('LIBRSVG_MINOR_VERSION', version_parts[1].to_int())
features_conf.set('LIBRSVG_MICRO_VERSION', micro_version)
features_conf.set('PACKAGE_VERSION', meson.project_version())

features_header = configure_file(
  input: 'librsvg-features.h.in',
  output: 'librsvg-features.h',
  configuration: features_conf,
  install_dir: incdir,
)

enum_headers = [
  'rsvg.h',
  'rsvg-cairo.h',
]

enum_header = custom_target(
  'librsvg-enum-types.h',
  input: enum_headers,
  output: 'librsvg-enum-types.h',
  capture: true,
  command: [
    glib_mkenums,
    '--fhead',
    '#if !defined (__RSVG_RSVG_H_INSIDE__) && !defined (RSVG_COMPILATION)\n#warning "Including <librsvg/librsvg-enum-types.h> directly is deprecated."\n#endif\n\n#ifndef __LIBRSVG_ENUM_TYPES_H__\n#define __LIBRSVG_ENUM_TYPES_H__\n\n#include <glib-object.h>\n\nG_BEGIN_DECLS\n',
    '--fprod',
    '/* enumerations from "@filename@" */\n',
    '--vhead',
    'GType @enum_name@_get_type (void);\n#define RSVG_TYPE_@ENUMSHORT@ (@enum_name@_get_type())\n',
    '--ftail',
    'G_END_DECLS\n\n#endif /* __LIBRSVG_ENUM_TYPES_H__ */',
    '@INPUT@',
  ],
  install: true,
  install_dir: incdir,
)

enum_source = custom_target(
  'librsvg-enum-types.c',
  input: enum_headers,
  output: 'librsvg-enum-types.c',
  capture: true,
  command: [
    glib_mkenums,
    '--fhead',
    '#include "rsvg.h"',
    '--fprod',
    '\n/* enumerations from "@filename@" */',
    '--vhead',
    'GType\n@enum_name@_get_type (void)\n{\n  static GType etype = 0;\n  if (etype == 0) {\n    static const G@Type@Value values[] = {',
    '--vprod',
    '      { @VALUENAME@, "@VALUENAME@", "@valuenick@" },',
    '--vtail',
    '      { 0, NULL, NULL }\n    };\n    etype = g_@type@_register_static ("@EnumName@", values);\n  }\n  return etype;\n}\n',
    '@INPUT@',
  ],
)

public_headers = [
  'rsvg.h',
  'rsvg-cairo.h',
]

install_headers(public_headers, subdir: 'librsvg-2.0/librsvg')

lib_sources = files(
  'librsvg-features.c',
  'rsvg-base-file-util.c',
  'rsvg-base.c',
  'rsvg-cairo-clip.c',
  'rsvg-cairo-draw.c',
  'rsvg-cairo-render.c',
  'rsvg-cond.c',
  'rsvg-css.c',
  'rsvg-defs.c',
  'rsvg-file-util.c',
  'rsvg-filter.c',
  'rsvg-gobject.c',
  'rsvg-image.c',
  'rsvg-io.c',
  'rsvg-marker.c',
  'rsvg-mask.c',
  'rsvg-paint-server.c',
  'rsvg-path.c',
  'rsvg-shapes.c',
  'rsvg-size-callback.c',
  'rsvg-structure.c',
  'rsvg-styles.c',
  'rsvg-text.c',
  'rsvg-xml.c',
  'rsvg.c',
)

lib_deps = [
  glib_dep,
  gobject_dep,
  gio_dep,
  gdk_pixbuf_dep,
  libxml_dep,
  pangocairo_dep,
  pangoft2_dep,
  cairo_dep,
  cairo_png_dep,
  croco_dep,
  fontconfig_dep,
]

if m_dep.found()
  lib_deps += m_dep
endif

lib_includes = [
  config_inc,
  include_directories('.'),
]

lib_c_args = [
  '-DG_LOG_DOMAIN="librsvg"',
  '-DRSVG_DISABLE_DEPRECATION_WARNINGS',
  '-DRSVG_COMPILATION',
  '-DSRCDIR="@0@"'.format(meson.project_source_root()),
]

librsvg_lib = library(
  'rsvg-' + api_major,
  sources: lib_sources + [enum_source, enum_header],
  c_args: lib_c_args,
  include_directories: lib_includes,
  dependencies: lib_deps,
  version: meson.project_version(),
  soversion: api_major,
  link_args: bsymbolic_link_args,
  install: true,
)

librsvg_dep = declare_dependency(
  link_with: librsvg_lib,
  include_directories: lib_includes,
  dependencies: lib_deps,
  sources: [enum_header, features_header],
)

pkgconfig.generate(
  libraries: librsvg_lib,
  version: meson.project_version(),
  name: 'librsvg-2.0',
  filebase: 'librsvg-2.0',
  description: 'Library to render SVG files',
  subdirs: 'librsvg-2.0',
  requires: [
    'gio-2.0',
    'gdk-pixbuf-2.0',
    'glib-2.0',
    'gobject-2.0',
    'libxml-2.0',
    'pangoft2',
    'pangocairo',
    'cairo',
    'cairo-png',
    'libcroco-0.6',
    'fontconfig',
  ],
)

if gi_dep.found()
  gir_extra_args = [
    '--c-include=librsvg/rsvg.h',
    '--symbol-prefix=rsvg',
    '--identifier-prefix=Rsvg',
    '--warn-all',
  ]

  gnome.generate_gir(
    librsvg_lib,
    sources: lib_sources + enum_headers + [enum_source],
    nsversion: api_version,
    namespace: 'Rsvg',
    include_directories: lib_includes,
    includes: [
      'GObject-2.0',
      'Gio-2.0',
      'cairo-1.0',
      'GdkPixbuf-2.0',
    ],
    export_packages: 'librsvg-2.0',
    header: 'librsvg/rsvg.h',
    install: true,
    install_dir_gir: join_paths(get_option('datadir'), 'gir-1.0'),
    install_dir_typelib: join_paths(get_option('libdir'), 'girepository-1.0'),
    extra_args: gir_extra_args,
    dependencies: lib_deps,
  )
endif

meson.override_dependency('librsvg-2.0', librsvg_dep)
