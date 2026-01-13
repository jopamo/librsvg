test_includes = [src_inc, config_inc]
python3 = find_program('python3')

common_test_sources = ['test-utils.c']

test_programs = {
  'ci-workflow': ['ci-workflow.c'],
  'test-setup-config': ['test-setup-config.c'],
  'rsvg-test': ['rsvg-test.c'],
  'loading': ['loading.c'],
  'crash': ['crash.c'],
  'styles': ['styles.c'],
  'render-crash': ['render-crash.c'],
  'dimensions': ['dimensions.c'],
  'errors': ['errors.c'],
  'path-builder': ['path-builder.c'],
  'security-check': ['security-check.c'],
  'vulnerability-check': ['vulnerability_check.c'],
  'xss-checks': ['xss_checks.c'],
  'api': ['api.c'],
  'restyle': ['restyle.c'],
  'style-dump': ['style-dump.c'],
}

if get_option('ci')
  test_programs += {'ci-option': ['ci-option.c']}
endif

test_env_common = environment({
  'G_TEST_SRCDIR': meson.current_source_dir(),
  'G_TEST_BUILDDIR': meson.current_build_dir(),
  'G_TEST_DIST': meson.current_source_dir(),
})

targets_env = test_env_common
run_clang_tidy = find_program('run-clang-tidy', required: false)
clang_tidy = find_program('clang-tidy', required: false)
if run_clang_tidy.found() and clang_tidy.found()
  targets_env.set('RSVG_EXPECT_CLANG_TIDY', '1')
endif
scan_build = find_program('scan-build', required: false)
if scan_build.found()
  targets_env.set('RSVG_EXPECT_SCAN_BUILD', '1')
endif
test(
  'build-targets',
  python3,
  args: files('check-build-targets.py'),
  env: targets_env,
  timeout: 60,
)

asan_env = environment({
  'ASAN_OPTIONS': 'halt_on_error=1:detect_leaks=1:strict_string_checks=1',
})

ubsan_env = environment({
  'UBSAN_OPTIONS': 'halt_on_error=1:print_stacktrace=1',
})

asan_ubsan_env = environment({
  'ASAN_OPTIONS': 'halt_on_error=1:detect_leaks=1:strict_string_checks=1',
  'UBSAN_OPTIONS': 'halt_on_error=1:print_stacktrace=1',
})

glib_debug_env = environment({
  'G_SLICE': 'always-malloc',
  'G_DEBUG': 'gc-friendly',
})

add_test_setup('asan', env: asan_env)
add_test_setup('ubsan', env: ubsan_env)
add_test_setup('asan_ubsan', env: asan_ubsan_env)
add_test_setup('glib_debug', env: glib_debug_env)

foreach name, sources : test_programs
  exe = executable(
    name,
    sources + common_test_sources,
    include_directories: test_includes,
    dependencies: [
      librsvg_dep,
    ],
    install: false,
  )

  test_env = test_env_common
  if name == 'security-check' or name == 'render-crash' or name == 'rsvg-test'
    test_env.set('ASAN_OPTIONS', 'detect_leaks=0')
    test_env.set('LSAN_OPTIONS', 'detect_leaks=0')
  endif

  test(name, exe, env: test_env, timeout: 120)
endforeach

if gtk4_dep.found()
  test_gtk4_exe = executable(
    'test-gtk4',
    ['test-gtk4.c'] + common_test_sources,
    include_directories: test_includes,
    dependencies: [
      librsvg_dep,
      gtk4_dep,
    ],
    install: false,
  )
  test('test-gtk4', test_gtk4_exe, env: test_env_common, timeout: 120)
endif

if get_option('gdk_pixbuf_loader') and get_option('dev_tools')
  rsvg_loader_exe = executable(
    'rsvg-loader-test-bin',
    '../gdk-pixbuf-loader/test.c',
    include_directories: test_includes,
    dependencies: [
      gdk_pixbuf_dep,
      glib_dep,
    ],
    install: false,
  )

  test_loader_py = find_program('test-loader.py')
  
  test(
    'loader-svg',
    test_loader_py,
    args: [
      rsvg_loader_exe.full_path(),
      loader.full_path(),
      files('fixtures/loading/gnome-cool.svg'),
      'gnome-cool.png'
    ],
    depends: [rsvg_loader_exe, loader],
    env: test_env_common,
  )

  test(
    'loader-svgz',
    test_loader_py,
    args: [
      rsvg_loader_exe.full_path(),
      loader.full_path(),
      files('fixtures/loading/gnome-cool.svgz'),
      'gnome-cool-z.png'
    ],
    depends: [rsvg_loader_exe, loader],
    env: test_env_common,
  )

  test(
    'loader-security',
    test_loader_py,
    args: [
      rsvg_loader_exe.full_path(),
      loader.full_path(),
      files('fixtures/errors/515-too-many-elements.svgz'),
      'too-many-elements.png'
    ],
    depends: [rsvg_loader_exe, loader],
    env: test_env_common,
    should_fail: true,
    is_parallel: false,
    timeout: 120,
  )
endif

if get_option('fuzzing')
  fuzz_c_args = cc.get_supported_arguments([
    '-fsanitize=fuzzer,address,undefined',
  ])
  
  fuzz_programs = {
    'fuzz-css': ['fuzz/fuzz-css.c'],
    'fuzz-load-render': ['fuzz/fuzz-load-render.c'],
    'fuzz-path': ['fuzz/fuzz-path.c'],
  }
  
  foreach name, sources : fuzz_programs
    c_args = []
    link_args = []
    if fuzz_c_args.length() > 0
      c_args += fuzz_c_args
      link_args += fuzz_c_args
      c_args += '-DRSVG_FUZZING_ENGINE'
    endif

    executable(
      name,
      sources,
      include_directories: test_includes,
      dependencies: [
        librsvg_private_dep,
      ],
      c_args: c_args,
      link_args: link_args,
      install: false,
    )
  endforeach
endif
