/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim: set ts=4 nowrap ai expandtab sw=4: */

#include "config.h"

#include <glib.h>
#include "rsvg.h"
#include "test-utils.h"

static void test_error_quark(void) {
    g_assert_cmpint(RSVG_ERROR, !=, 0);
    g_assert_cmpstr(g_quark_to_string(RSVG_ERROR), ==, "rsvg-error-quark");
}

static void test_handle_new_from_file_non_existent(void) {
    GError* error = NULL;
    RsvgHandle* handle;

    handle = rsvg_handle_new_from_file("non-existent-file.svg", &error);
    g_assert_null(handle);
    g_assert_nonnull(error);
    /* GIO error domain usually, but let's check it's set */
    g_error_free(error);
}

static void test_sticky_error(void) {
    RsvgHandle* handle;
    GError* error = NULL;
    gboolean result;

    handle = rsvg_handle_new();

    /* Valid SVG start */
    result = rsvg_handle_write(handle, (const guchar*)"<svg>", 5, &error);
    g_assert_true(result);
    g_assert_no_error(error);

    /* Definitely invalid data that should trigger a SAX error */
    /* Using a very broken XML */
    result = rsvg_handle_write(handle, (const guchar*)"<svg><<<<<", 9, &error);
    if (result) {
        /* If it didn't fail yet, try to close it which should fail */
        result = rsvg_handle_close(handle, &error);
    }

    g_assert_false(result);
    g_assert_nonnull(error);
    g_assert_cmpint(error->domain, ==, RSVG_ERROR);
    g_clear_error(&error);

    /* Subsequent write should also fail if errors are sticky */
    result = rsvg_handle_write(handle, (const guchar*)"<rect/>", 7, &error);
    g_assert_false(result);
    g_assert_nonnull(error);
    g_clear_error(&error);

    g_object_unref(handle);
}

static void test_double_close(void) {
    RsvgHandle* handle;
    GError* error = NULL;
    gboolean result;

    handle = rsvg_handle_new();
    rsvg_handle_write(handle, (const guchar*)"<svg/>", 6, NULL);

    result = rsvg_handle_close(handle, &error);
    g_assert_true(result);
    g_assert_no_error(error);

    /* Second close should be idempotent and return TRUE */
    result = rsvg_handle_close(handle, &error);
    g_assert_true(result);
    g_assert_no_error(error);

    g_object_unref(handle);
}

static void test_handle_new_from_data_invalid(void) {
    GError* error = NULL;
    RsvgHandle* handle;

    handle = rsvg_handle_new_from_data((const guint8*)"not svg", 7, &error);
    g_assert_null(handle);
    g_assert_nonnull(error);
    g_assert_cmpint(error->domain, ==, RSVG_ERROR);
    g_error_free(error);
}

static void test_handle_get_dimensions_no_base_uri(void) {
    RsvgHandle* handle;
    RsvgDimensionData dim;

    handle = rsvg_handle_new();
    rsvg_handle_write(handle, (const guchar*)"<svg width='100' height='100'></svg>", 36, NULL);
    rsvg_handle_close(handle, NULL);

    rsvg_handle_get_dimensions(handle, &dim);
    g_assert_cmpint(dim.width, ==, 100);
    g_assert_cmpint(dim.height, ==, 100);

    g_object_unref(handle);
}

static void test_handle_has_sub_invalid(void) {
    RsvgHandle* handle;

    handle = rsvg_handle_new();
    rsvg_handle_write(handle, (const guchar*)"<svg><rect id='foo'/></svg>", 27, NULL);
    rsvg_handle_close(handle, NULL);

    g_assert_true(rsvg_handle_has_sub(handle, "#foo"));
    g_assert_false(rsvg_handle_has_sub(handle, "#bar"));
    g_assert_false(rsvg_handle_has_sub(handle, NULL));
    g_assert_false(rsvg_handle_has_sub(handle, ""));

    g_object_unref(handle);
}

int main(int argc, char** argv) {
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/api/error_quark", test_error_quark);
    g_test_add_func("/api/handle_new_from_file_non_existent", test_handle_new_from_file_non_existent);
    g_test_add_func("/api/handle_new_from_data_invalid", test_handle_new_from_data_invalid);
    g_test_add_func("/api/sticky_error", test_sticky_error);
    g_test_add_func("/api/double_close", test_double_close);
    g_test_add_func("/api/handle_get_dimensions_no_base_uri", test_handle_get_dimensions_no_base_uri);
    g_test_add_func("/api/handle_has_sub_invalid", test_handle_has_sub_invalid);

    return g_test_run();
}
