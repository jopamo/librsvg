test_includes = [src_inc, config_inc]

common_test_sources = ['test-utils.c']

test_programs = {
  'rsvg-test': ['rsvg-test.c'],
  'loading': ['loading.c'],
  'crash': ['crash.c'],
  'styles': ['styles.c'],
  'render-crash': ['render-crash.c'],
  'dimensions': ['dimensions.c'],
  'errors': ['errors.c'],
  'security-check': ['security-check.c'],
  'vulnerability-check': ['vulnerability_check.c'],
}

test_env_common = environment({
  'G_TEST_SRCDIR': meson.current_source_dir(),
  'G_TEST_BUILDDIR': meson.current_build_dir(),
  'G_TEST_DIST': meson.current_source_dir(),
})

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
