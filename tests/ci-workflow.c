/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim: set sw=4 sts=4 ts=4 expandtab: */

#include "config.h"

#include <glib.h>
#include <string.h>

static gchar* load_workflow_contents(void) {
    const gchar* srcdir = g_getenv("G_TEST_SRCDIR");
    gchar* workflow_path = NULL;
    gchar* contents = NULL;
    GError* error = NULL;

    g_assert_nonnull(srcdir);

    workflow_path = g_build_filename(srcdir, "..", ".github", "workflows", "ci.yml", NULL);
    g_file_get_contents(workflow_path, &contents, NULL, &error);
    g_assert_no_error(error);
    g_free(workflow_path);

    return contents;
}

static void assert_contains_all(const gchar* contents, const gchar* const* needles) {
    for (size_t i = 0; needles[i] != NULL; i++) {
        g_assert_nonnull(strstr(contents, needles[i]));
    }
}

static void test_ci_workflow_has_expected_jobs(void) {
    gchar* contents = load_workflow_contents();
    const gchar* required[] = {
        "clang:", "gcc:", "asan-ubsan:", "debugoptimized:", "introspection:", "pixbuf-loader:", NULL,
    };

    assert_contains_all(contents, required);
    g_free(contents);
}

static void test_ci_workflow_has_expected_setup_args(void) {
    gchar* contents = load_workflow_contents();
    const gchar* required[] = {
        "meson setup build-clang",
        "meson setup build-gcc",
        "-Db_sanitize=address,undefined",
        "-Db_lundef=false",
        "--buildtype=debugoptimized",
        "-Dintrospection=${{ matrix.enabled }}",
        "-Dgdk_pixbuf_loader=${{ matrix.enabled }}",
        NULL,
    };

    assert_contains_all(contents, required);
    g_free(contents);
}

int main(int argc, char** argv) {
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/ci/workflow/jobs", test_ci_workflow_has_expected_jobs);
    g_test_add_func("/ci/workflow/setup-args", test_ci_workflow_has_expected_setup_args);

    return g_test_run();
}
