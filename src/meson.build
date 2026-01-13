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
]

enum_header = gnome.mkenums(
  'librsvg-enum-types.h',
  sources: enum_headers,
  identifier_prefix: 'Rsvg',
  symbol_prefix: 'rsvg',
  fhead: '''#if !defined (__RSVG_RSVG_H_INSIDE__) && !defined (RSVG_COMPILATION)
#warning "Including <librsvg/librsvg-enum-types.h> directly is deprecated."
#endif

#ifndef __LIBRSVG_ENUM_TYPES_H__
#define __LIBRSVG_ENUM_TYPES_H__

#include <glib-object.h>

G_BEGIN_DECLS
''',
  fprod: '/* enumerations from "@filename@" */',
  vhead: '''GType @enum_name@_get_type (void);
#define RSVG_TYPE_@ENUMSHORT@ (@enum_name@_get_type())
''',
  ftail: '''G_END_DECLS

#endif /* __LIBRSVG_ENUM_TYPES_H__ */
''',
  install_header: true,
  install_dir: incdir,
)

enum_source = gnome.mkenums(
  'librsvg-enum-types.c',
  sources: enum_headers,
  identifier_prefix: 'Rsvg',
  symbol_prefix: 'rsvg',
  fhead: '#include "rsvg.h"',
  fprod: '''
/* enumerations from "@filename@" */''',
  vhead: '''GType
@enum_name@_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const G@Type@Value values[] = {''',
  vprod: '      { @VALUENAME@, "@VALUENAME@", "@valuenick@" },',
  vtail: '''      { 0, NULL, NULL }
    };
    etype = g_@type@_register_static ("@EnumName@", values);
  }
  return etype;
}
''',
)

public_headers = [
  'rsvg.h',
  'rsvg-cairo.h',
  'rsvg-deprecated.h',
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
  'rsvg-css-engine.c',
  'rsvg-css-engine-croco.c',
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
  css_dep,
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

librsvg_lib = both_libraries(
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
  link_with: librsvg_lib.get_shared_lib(),
  include_directories: lib_includes,
  dependencies: lib_deps,
  sources: [enum_header, features_header],
)

librsvg_private_dep = declare_dependency(
  link_with: librsvg_lib.get_static_lib(),
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
