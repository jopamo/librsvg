/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim: set sw=4 sts=4 ts=4 expandtab: */

#include "config.h"

#include <glib.h>
#include <string.h>

static gchar* load_tests_meson_contents(void) {
    const gchar* srcdir = g_getenv("G_TEST_SRCDIR");
    gchar* meson_path = NULL;
    gchar* contents = NULL;
    GError* error = NULL;

    g_assert_nonnull(srcdir);

    meson_path = g_build_filename(srcdir, "meson.build", NULL);
    g_file_get_contents(meson_path, &contents, NULL, &error);
    g_assert_no_error(error);
    g_free(meson_path);

    return contents;
}

static void assert_contains_all(const gchar* contents, const gchar* const* needles) {
    for (size_t i = 0; needles[i] != NULL; i++) {
        g_assert_nonnull(strstr(contents, needles[i]));
    }
}

static void test_meson_has_sanitizer_setups(void) {
    gchar* contents = load_tests_meson_contents();
    const gchar* required[] = {
        "add_test_setup('asan'",
        "add_test_setup('ubsan'",
        "add_test_setup('asan_ubsan'",
        "ASAN_OPTIONS': 'halt_on_error=1:detect_leaks=1:strict_string_checks=1",
        "UBSAN_OPTIONS': 'halt_on_error=1:print_stacktrace=1",
        NULL,
    };

    assert_contains_all(contents, required);
    g_free(contents);
}

static void test_meson_has_glib_debug_setup(void) {
    gchar* contents = load_tests_meson_contents();
    const gchar* required[] = {
        "add_test_setup('glib_debug'",
        "G_SLICE': 'always-malloc",
        "G_DEBUG': 'gc-friendly",
        NULL,
    };

    assert_contains_all(contents, required);
    g_free(contents);
}

int main(int argc, char** argv) {
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/meson/test-setup/sanitizers", test_meson_has_sanitizer_setups);
    g_test_add_func("/meson/test-setup/glib-debug", test_meson_has_glib_debug_setup);

    return g_test_run();
}
