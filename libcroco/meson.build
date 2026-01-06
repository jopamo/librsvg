libcroco_sources = [
  'src/cr-additional-sel.c',
  'src/cr-attr-sel.c',
  'src/cr-cascade.c',
  'src/cr-declaration.c',
  'src/cr-doc-handler.c',
  'src/cr-enc-handler.c',
  'src/cr-fonts.c',
  'src/cr-input.c',
  'src/cr-num.c',
  'src/cr-om-parser.c',
  'src/cr-parser.c',
  'src/cr-parsing-location.c',
  'src/cr-prop-list.c',
  'src/cr-pseudo.c',
  'src/cr-rgb.c',
  'src/cr-sel-eng.c',
  'src/cr-selector.c',
  'src/cr-simple-sel.c',
  'src/cr-statement.c',
  'src/cr-string.c',
  'src/cr-style.c',
  'src/cr-stylesheet.c',
  'src/cr-term.c',
  'src/cr-tknzr.c',
  'src/cr-token.c',
  'src/cr-utils.c',
]

libcroco_c_args = cc.get_supported_arguments([
  '-Wmissing-prototypes',
  '-Wstrict-prototypes',
  '-Wnested-externs',
  '-Wpointer-arith',
  '-Wno-unused-parameter',
  '-Wsign-compare',
  '-Wtype-limits',
])

libcroco_lib = static_library('croco',
  libcroco_sources,
  dependencies: [glib_dep, libxml_dep],
  include_directories: [include_directories('src'), config_inc],
  c_args: libcroco_c_args,
)

croco_dep = declare_dependency(
  link_with: libcroco_lib,
  include_directories: include_directories('src'),
)

libcroco_term_test = executable(
  'libcroco-term-test',
  files('tests/test-term.c'),
  include_directories: include_directories('src'),
  dependencies: [croco_dep, glib_dep, libxml_dep],
  install: false,
)

test('libcroco-term', libcroco_term_test)

libcroco_css_injection_test = executable(
  'libcroco-css-injection-test',
  files('tests/test-css-injection.c'),
  include_directories: include_directories('src'),
  dependencies: [croco_dep, glib_dep, libxml_dep],
  install: false,
)

test('libcroco-css-injection', libcroco_css_injection_test)
