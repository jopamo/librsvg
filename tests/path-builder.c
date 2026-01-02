/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim: set sw=4 sts=4 ts=4 expandtab: */

#include "config.h"

#include <glib.h>
#include <string.h>
#include <cairo.h>

#include "rsvg.h"

static void test_parse_simple_path(void) {
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"10\" height=\"10\">"
        "<path d=\"M0 0 L10 10 Z\" />"
        "</svg>";
    GError* error = NULL;
    RsvgHandle* handle = rsvg_handle_new_from_data((const guint8*)svg, strlen(svg), &error);
    RsvgDimensionData dims;
    cairo_surface_t* surface = NULL;
    cairo_t* cr = NULL;

    g_assert_no_error(error);
    g_assert_nonnull(handle);

    rsvg_handle_get_dimensions(handle, &dims);
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, dims.width, dims.height);
    cr = cairo_create(surface);

    g_assert(rsvg_handle_render_cairo(handle, cr));

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    g_object_unref(handle);
}

int main(int argc, char** argv) {
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/path/builder/parse-simple", test_parse_simple_path);

    return g_test_run();
}
