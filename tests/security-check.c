#include <config.h>
#include <stdlib.h>
#include <locale.h>
#include <glib.h>
#include <cairo.h>
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

int main(int argc, char** argv) {
    g_test_init(&argc, &argv, NULL);
    /* test_utils_setup might not be needed if we don't use the complex fixture finder,
       but let's see test-utils.c to be sure. */

    g_test_add_func("/security/xinclude-disabled", test_xinclude_disabled);
    g_test_add_func("/security/tref-removed", test_tref_removed);

    return g_test_run();
}
