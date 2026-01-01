#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <cairo.h>
#include "rsvg.h"
#include "test-utils.h"

static void test_script_tag_ignored(void) {
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\">"
        "  <script>alert('XSS')</script>"
        "  <rect width=\"100\" height=\"100\" fill=\"red\"/>"
        "</svg>";

    GError* error = NULL;
    RsvgHandle* handle = rsvg_handle_new_from_data((const guint8*)svg, strlen(svg), &error);

    /* It should load without error, ignoring the script tag */
    g_assert_no_error(error);
    g_assert(handle != NULL);

    /* Render to ensure no side effects/crashes */
    RsvgDimensionData dims;
    rsvg_handle_get_dimensions(handle, &dims);
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, dims.width, dims.height);
    cairo_t* cr = cairo_create(surface);

    gboolean res = rsvg_handle_render_cairo(handle, cr);
    g_assert(res);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    g_object_unref(handle);
}

static void test_javascript_url_image(void) {
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">"
        "  <image xlink:href=\"javascript:alert('XSS')\" width=\"100\" height=\"100\"/>"
        "</svg>";

    GError* error = NULL;
    RsvgHandle* handle = rsvg_handle_new_from_data((const guint8*)svg, strlen(svg), &error);

    /* It should load, but the image should fail to load silently or with a warning, not execute */
    g_assert_no_error(error);
    g_assert(handle != NULL);

    RsvgDimensionData dims;
    rsvg_handle_get_dimensions(handle, &dims);
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, dims.width, dims.height);
    cairo_t* cr = cairo_create(surface);

    /* Render */
    rsvg_handle_render_cairo(handle, cr);
    /* We don't assert(res) here because librsvg might return FALSE when an image fails to load.
       The important thing is that it doesn't execute the script (which would be visible/auditable if we had a JS
       engine) and doesn't crash. */

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    g_object_unref(handle);
}

static void test_foreign_object_ignored(void) {
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\">"
        "  <foreignObject width=\"100\" height=\"100\">"
        "    <body xmlns=\"http://www.w3.org/1999/xhtml\">"
        "      <script>alert('XSS')</script>"
        "    </body>"
        "  </foreignObject>"
        "</svg>";

    GError* error = NULL;
    RsvgHandle* handle = rsvg_handle_new_from_data((const guint8*)svg, strlen(svg), &error);

    g_assert_no_error(error);
    g_assert(handle != NULL);

    RsvgDimensionData dims;
    rsvg_handle_get_dimensions(handle, &dims);
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, dims.width, dims.height);
    cairo_t* cr = cairo_create(surface);

    gboolean res = rsvg_handle_render_cairo(handle, cr);
    /* ForeignObject might fail if unsupported, which is safe. */
    (void)res;

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    g_object_unref(handle);
}

static void test_event_handlers_ignored(void) {
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" onload=\"alert('XSS')\">"
        "  <rect width=\"100\" height=\"100\" fill=\"blue\" onclick=\"alert('click')\"/>"
        "</svg>";

    GError* error = NULL;
    RsvgHandle* handle = rsvg_handle_new_from_data((const guint8*)svg, strlen(svg), &error);

    g_assert_no_error(error);
    g_assert(handle != NULL);

    RsvgDimensionData dims;
    rsvg_handle_get_dimensions(handle, &dims);
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, dims.width, dims.height);
    cairo_t* cr = cairo_create(surface);

    gboolean res = rsvg_handle_render_cairo(handle, cr);
    g_assert(res);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    g_object_unref(handle);
}

int main(int argc, char** argv) {
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/security/script-tag-ignored", test_script_tag_ignored);
    g_test_add_func("/security/javascript-url-image", test_javascript_url_image);
    g_test_add_func("/security/foreign-object-ignored", test_foreign_object_ignored);
    g_test_add_func("/security/event-handlers-ignored", test_event_handlers_ignored);

    return g_test_run();
}
