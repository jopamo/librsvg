#include <config.h>
#include <stdlib.h>
#include <locale.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <cairo.h>
#include <stdint.h>
#include <fontconfig/fontconfig.h>
#include "rsvg.h"
#include "test-utils.h"

static void test_xinclude_disabled(void) {
    char* file_path = g_build_filename(test_utils_get_test_data_path(), "xinclude.svg", NULL);
    GError* error = NULL;
    RsvgHandle* handle = rsvg_handle_new_from_file(file_path, &error);

    /* Since xi:include is disabled/unknown, it should load fine but ignore the inclusion. */
    g_assert_no_error(error);
    g_assert(handle != NULL);

    RsvgDimensionData dims;
    rsvg_handle_get_dimensions(handle, &dims);
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, dims.width, dims.height);
    cairo_t* cr = cairo_create(surface);

    /* Rendering should not crash */
    rsvg_handle_render_cairo(handle, cr);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    g_object_unref(handle);
    g_free(file_path);
}

static void test_tref_removed(void) {
    char* file_path = g_build_filename(test_utils_get_test_data_path(), "tref.svg", NULL);
    GError* error = NULL;
    RsvgHandle* handle = rsvg_handle_new_from_file(file_path, &error);

    /* Should load fine (tref is unknown element) */
    g_assert_no_error(error);
    g_assert(handle != NULL);

    RsvgDimensionData dims;
    rsvg_handle_get_dimensions(handle, &dims);
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, dims.width, dims.height);
    cairo_t* cr = cairo_create(surface);

    /* Rendering should not crash */
    rsvg_handle_render_cairo(handle, cr);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    g_object_unref(handle);
    g_free(file_path);
}

static guint32 get_pixel_argb32(cairo_surface_t* surface, int x, int y) {
    unsigned char* data = NULL;
    int stride = 0;
    guint32* row = NULL;

    cairo_surface_flush(surface);

    data = cairo_image_surface_get_data(surface);
    stride = cairo_image_surface_get_stride(surface);
    row = (guint32*)(data + y * stride);

    return row[x];
}

static void test_xinclude_ignored(void) {
    GError* error = NULL;
    char* tmpdir = g_dir_make_tmp("rsvg-xinclude-XXXXXX", &error);
    char* include_path = NULL;
    char* main_path = NULL;
    RsvgHandle* handle = NULL;
    cairo_surface_t* surface = NULL;
    cairo_t* cr = NULL;
    guint32 pixel = 0;
    const char* included_svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"10\" height=\"10\">"
        "<rect width=\"10\" height=\"10\" fill=\"black\" />"
        "</svg>";
    const char* main_svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xi=\"http://www.w3.org/2001/XInclude\" width=\"10\" "
        "height=\"10\">"
        "<xi:include href=\"included.svg\" parse=\"xml\"/>"
        "</svg>";

    g_assert_no_error(error);
    g_assert_nonnull(tmpdir);

    include_path = g_build_filename(tmpdir, "included.svg", NULL);
    main_path = g_build_filename(tmpdir, "main.svg", NULL);

    g_assert(g_file_set_contents(include_path, included_svg, -1, &error));
    g_assert_no_error(error);
    g_assert(g_file_set_contents(main_path, main_svg, -1, &error));
    g_assert_no_error(error);

    handle = rsvg_handle_new_from_file(main_path, &error);
    g_assert_no_error(error);
    g_assert_nonnull(handle);

    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 10, 10);
    cr = cairo_create(surface);
    g_assert(rsvg_handle_render_cairo(handle, cr));

    pixel = get_pixel_argb32(surface, 5, 5);
    g_assert_cmpuint(pixel, ==, 0);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    g_object_unref(handle);

    g_remove(main_path);
    g_remove(include_path);
    g_rmdir(tmpdir);

    g_free(main_path);
    g_free(include_path);
    g_free(tmpdir);
}

static void test_external_entity_blocked(void) {
    GError* error = NULL;
    char* tmpdir = g_dir_make_tmp("rsvg-xxe-XXXXXX", &error);
    char* secret_path = NULL;
    char* svg_path = NULL;
    RsvgHandle* handle = NULL;
    const char* svg =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE svg [\n"
        "  <!ENTITY secret SYSTEM \"secret.txt\">\n"
        "]>\n"
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"10\" height=\"10\">\n"
        "  <text x=\"0\" y=\"10\">&secret;</text>\n"
        "</svg>\n";

    g_assert_no_error(error);
    g_assert_nonnull(tmpdir);

    secret_path = g_build_filename(tmpdir, "secret.txt", NULL);
    svg_path = g_build_filename(tmpdir, "xxe.svg", NULL);

    g_assert(g_file_set_contents(secret_path, "TOPSECRET", -1, &error));
    g_assert_no_error(error);
    g_assert_cmpint(g_chmod(secret_path, 0), ==, 0);

    g_assert(g_file_set_contents(svg_path, svg, -1, &error));
    g_assert_no_error(error);

    handle = rsvg_handle_new_from_file(svg_path, &error);
    g_assert_null(handle);
    g_assert_nonnull(error);
    g_assert(g_error_matches(error, RSVG_ERROR, RSVG_ERROR_FAILED));
    g_assert_cmpstr(error->message, ==, "External entities are not allowed");

    g_chmod(secret_path, 0600);
    g_error_free(error);
    g_remove(svg_path);
    g_remove(secret_path);
    g_rmdir(tmpdir);

    g_free(svg_path);
    g_free(secret_path);
    g_free(tmpdir);
}

int main(int argc, char** argv) {
    int result;
    g_test_init(&argc, &argv, NULL);
    /* test_utils_setup might not be needed if we don't use the complex fixture finder,
       but let's see test-utils.c to be sure. */

    g_test_add_func("/security/xinclude-disabled", test_xinclude_disabled);
    g_test_add_func("/security/xinclude-ignored", test_xinclude_ignored);
    g_test_add_func("/security/tref-removed", test_tref_removed);
    g_test_add_func("/security/external-entity-blocked", test_external_entity_blocked);

    result = g_test_run();

    rsvg_cleanup();
    FcFini();

    return result;
}
