/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim: set ts=4 nowrap ai expandtab sw=4: */

#include "config.h"

#include <gtk/gtk.h>
#include "rsvg.h"
#include "test-utils.h"

static void test_gtk4_recolor_symbolic(void) {
    RsvgHandle* handle;
    GError* error = NULL;
    /* A simple symbolic SVG */
    const char* svg_data =
        "<svg width='16' height='16'>"
        "  <rect class='warning' width='16' height='16' fill='currentColor'/>"
        "</svg>";
    /* CSS to color it yellow (warning color) */
    const char* css_data = ".warning { color: #ffff00; }";

    handle = rsvg_handle_new_from_data((const guint8*)svg_data, strlen(svg_data), &error);
    g_assert_no_error(error);
    g_assert_nonnull(handle);

    /* Inject stylesheet */
    rsvg_handle_set_stylesheet(handle, (const guint8*)css_data, strlen(css_data), &error);
    g_assert_no_error(error);

    /* Render to a cairo surface */
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 16, 16);
    cairo_t* cr = cairo_create(surface);
    rsvg_handle_render_cairo(handle, cr);
    cairo_destroy(cr);

    /* Wrap in GdkTexture (GTK4 way) */
    /* We use gdk_memory_texture_new to wrap the cairo surface data */
    GBytes* bytes = g_bytes_new_static(cairo_image_surface_get_data(surface), 16 * 16 * 4);
    GdkTexture* texture = gdk_memory_texture_new(16, 16, GDK_MEMORY_DEFAULT, bytes, 16 * 4);
    g_assert_nonnull(texture);

    /* Verify color of the texture by downloading it back */
    guchar pixels[16 * 16 * 4];
    gdk_texture_download(texture, pixels, 16 * 4);

    /* Should be yellow: R=255, G=255, B=0.
       Note: GDK_MEMORY_DEFAULT is usually BGRA or RGBA depending on endianness.
       Cairo ARGB32 is native-endian.
    */

    /* Cairo ARGB32 on Little Endian is BGRA */
    /* In 0xAARRGGBB format: B is at index 0, G at 1, R at 2, A at 3 */
    g_assert_cmpint(pixels[1], ==, 255);  // G
    g_assert_cmpint(pixels[2], ==, 255);  // R
    g_assert_cmpint(pixels[0], ==, 0);    // B

    g_object_unref(texture);
    g_bytes_unref(bytes);
    cairo_surface_destroy(surface);
    g_object_unref(handle);
}

int main(int argc, char** argv) {
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/gtk4/recolor_symbolic", test_gtk4_recolor_symbolic);

    return g_test_run();
}
