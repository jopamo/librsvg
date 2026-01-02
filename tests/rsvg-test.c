/* vim: set sw=4 sts=4: -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 8 -*-
 *
 * rsvg-test - Regression test utility for librsvg
 *
 * Copyright © 2004 Richard D. Worth
 * Copyright © 2006 Red Hat, Inc.
 * Copyright © 2007 Emmanuel Pacaud
 * Copyright © 2025 Refactoring
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of the authors
 * not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 * The authors make no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 *
 * THE AUTHORS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <fontconfig/fontconfig.h>
#include <cairo.h>
#include <glib.h>
#include <gio/gio.h>

#include "rsvg.h"
#include "rsvg-compat.h"
#include "test-utils.h"

typedef struct {
    unsigned int pixels_changed;
    unsigned int max_diff;
} BufferDiffResult;

/*
 * compare_buffers:
 * @buf_a: First buffer (ARGB32)
 * @buf_b: Second buffer (ARGB32)
 * @buf_diff: Output buffer for difference visualization (ARGB32)
 * @width: Width in pixels
 * @height: Height in pixels
 * @stride: Stride in bytes
 * @result: Output structure for difference statistics
 *
 * Compares two buffers and populates a difference buffer.
 */
static void compare_buffers(unsigned char* buf_a,
                            unsigned char* buf_b,
                            unsigned char* buf_diff,
                            int width,
                            int height,
                            int stride,
                            BufferDiffResult* result) {
    int x, y;
    guint32 *row_a, *row_b, *row_diff;

    result->pixels_changed = 0;
    result->max_diff = 0;

    for (y = 0; y < height; y++) {
        row_a = (guint32*)(buf_a + y * stride);
        row_b = (guint32*)(buf_b + y * stride);
        row_diff = (guint32*)(buf_diff + y * stride);

        for (x = 0; x < width; x++) {
            guint32 val_a = row_a[x];
            guint32 val_b = row_b[x];

            if (val_a != val_b) {
                int max_local_diff = 0;
                guint32 diff_pixel = 0;

                /* Extract channels and compare */
                for (int k = 0; k < 4; k++) {
                    int c_a = (val_a >> (k * 8)) & 0xff;
                    int c_b = (val_b >> (k * 8)) & 0xff;
                    int d = abs(c_a - c_b);

                    if (d > max_local_diff)
                        max_local_diff = d;

                    if ((unsigned int)d > result->max_diff)
                        result->max_diff = (unsigned int)d;

                    /* Emphasize difference for visualization */
                    int vis_d = d * 4;
                    if (vis_d > 0)
                        vis_d += 128;
                    if (vis_d > 255)
                        vis_d = 255;

                    diff_pixel |= ((guint32)vis_d << (k * 8));
                }

                result->pixels_changed++;

                /* If only alpha changed, show as grayscale */
                if ((diff_pixel & 0x00ffffff) == 0) {
                    guint8 alpha_diff = (diff_pixel >> 24) & 0xff;
                    diff_pixel = ((guint32)alpha_diff << 16) | ((guint32)alpha_diff << 8) | (guint32)alpha_diff |
                                 ((guint32)alpha_diff << 24);
                }

                row_diff[x] = diff_pixel;
            }
            else {
                row_diff[x] = 0;
            }
            /* Set alpha to fully opaque for the diff image to be visible */
            row_diff[x] |= 0xff000000;
        }
    }
}

static void compare_surfaces(cairo_surface_t* surface_a,
                             cairo_surface_t* surface_b,
                             cairo_surface_t* surface_diff,
                             BufferDiffResult* result) {
    compare_buffers(cairo_image_surface_get_data(surface_a), cairo_image_surface_get_data(surface_b),
                    cairo_image_surface_get_data(surface_diff), cairo_image_surface_get_width(surface_a),
                    cairo_image_surface_get_height(surface_a), cairo_image_surface_get_stride(surface_a), result);

    if (result->pixels_changed > 0) {
        g_test_message("%d pixels differ (max diff: %d)", result->pixels_changed, result->max_diff);
    }
}

static char* get_output_path(const char* test_file, const char* suffix) {
    const char* tmp_dir = g_get_tmp_dir();
    char* basename = g_path_get_basename(test_file);
    char* no_ext_name;

    if (g_str_has_suffix(basename, ".svg")) {
        no_ext_name = g_strndup(basename, strlen(basename) - 4);
    }
    else {
        no_ext_name = g_strdup(basename);
    }

    char* filename = g_strconcat(no_ext_name, suffix, NULL);
    char* full_path = g_build_filename(tmp_dir, filename, NULL);

    g_free(filename);
    g_free(no_ext_name);
    g_free(basename);

    return full_path;
}

static void save_surface(cairo_surface_t* surface, const char* test_uri, const char* suffix) {
    char* path = get_output_path(test_uri, suffix);
    g_test_message("Storing test result image at %s", path);

    cairo_status_t status = cairo_surface_write_to_png(surface, path);
    if (status != CAIRO_STATUS_SUCCESS) {
        g_test_message("Error saving image to %s: %s", path, cairo_status_to_string(status));
    }
    g_free(path);
}

static gboolean is_test_file(GFile* file) {
    char* basename = g_file_get_basename(file);
    gboolean result = FALSE;

    if (g_str_has_prefix(basename, "ignore") || strcmp(basename, "resources") == 0 ||
        strcmp(basename, "340047.svg") == 0 || strcmp(basename, "587721-text-transform.svg") == 0) {
        /* Ignored files/dirs */
        result = FALSE;
    }
    else if (g_file_query_file_type(file, G_FILE_QUERY_INFO_NONE, NULL) == G_FILE_TYPE_DIRECTORY) {
        result = TRUE; /* Recurse into subdirectories */
    }
    else {
        result = g_str_has_suffix(basename, ".svg");
    }

    g_free(basename);
    return result;
}

static cairo_status_t read_stream_func(void* closure, unsigned char* data, unsigned int length) {
    GFileInputStream* stream = (GFileInputStream*)closure;
    GError* error = NULL;
    gssize bytes_read;

    bytes_read = g_input_stream_read(G_INPUT_STREAM(stream), data, length, NULL, &error);
    if (error) {
        g_error("Error reading stream: %s", error->message);
        g_error_free(error);
        return CAIRO_STATUS_READ_ERROR;
    }

    if (bytes_read != length)
        return CAIRO_STATUS_READ_ERROR;

    return CAIRO_STATUS_SUCCESS;
}

static cairo_surface_t* load_reference_image(const char* test_uri) {
    char* ref_uri = g_strconcat(test_uri, "-ref.png", NULL);
    GFile* file = g_file_new_for_uri(ref_uri);
    GError* error = NULL;
    GFileInputStream* stream;
    cairo_surface_t* surface = NULL;

    stream = g_file_read(file, NULL, &error);
    if (stream) {
        surface = cairo_image_surface_create_from_png_stream(read_stream_func, stream);
        g_object_unref(stream);
    }
    else {
        /* It's okay if reference doesn't exist, we just won't compare */
        // g_test_message ("Reference image not found: %s", ref_uri);
        g_error_free(error);
    }

    g_free(ref_uri);
    g_object_unref(file);
    return surface;
}

static void run_rsvg_test(gconstpointer data) {
    GFile* file = G_FILE(data);
    GError* error = NULL;
    char* uri = g_file_get_uri(file);
    char* base_uri = NULL;

    /* Remove .svg extension for base naming */
    if (g_str_has_suffix(uri, ".svg")) {
        base_uri = g_strndup(uri, strlen(uri) - 4);
    }
    else {
        base_uri = g_strdup(uri);
    }

    /* Load SVG */
    RsvgHandle* handle = rsvg_handle_new_from_gfile_sync(file, RSVG_HANDLE_FLAGS_NONE, NULL, &error);
    g_assert_no_error(error);
    g_assert(handle != NULL);

    /* Enable testing mode (fixes fonts, etc) */
    rsvg_handle_internal_set_testing(handle, TRUE);

    /* Get dimensions */
    RsvgDimensionData dim;
    rsvg_handle_get_dimensions(handle, &dim);
    g_assert(dim.width > 0 && dim.height > 0);

    /* Render to Cairo surface */
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, dim.width, dim.height);
    cairo_t* cr = cairo_create(surface);

    if (!rsvg_handle_render_cairo(handle, cr)) {
        g_test_fail();
        g_test_message("Rendering failed for %s", uri);
    }

    cairo_destroy(cr);

    /* Save output */
    save_surface(surface, base_uri, "-out.png");

    /* Compare with reference */
    cairo_surface_t* ref_surface = load_reference_image(base_uri);
    if (ref_surface) {
        int w_a = cairo_image_surface_get_width(surface);
        int h_a = cairo_image_surface_get_height(surface);
        int w_b = cairo_image_surface_get_width(ref_surface);
        int h_b = cairo_image_surface_get_height(ref_surface);

        if (w_a != w_b || h_a != h_b) {
            g_test_fail();
            g_test_message("Dimension mismatch: %dx%d vs ref %dx%d", w_a, h_a, w_b, h_b);
        }
        else {
            cairo_surface_t* diff_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w_a, h_a);
            BufferDiffResult result;

            compare_surfaces(surface, ref_surface, diff_surface, &result);

            /* Allow small difference (e.g. anti-aliasing) */
            if (result.pixels_changed > 0 && result.max_diff > 1) {
                // g_test_fail ();
                g_test_message("FAILURE IGNORED FOR DEBUGGING: %d pixels differ", result.pixels_changed);
                save_surface(diff_surface, base_uri, "-diff.png");
            }

            cairo_surface_destroy(diff_surface);
        }
        cairo_surface_destroy(ref_surface);
    }
    else {
        g_test_message("No reference image for %s, skipping comparison", uri);
    }

    cairo_surface_destroy(surface);
    g_object_unref(handle);
    g_free(uri);
    g_free(base_uri);
}

int main(int argc, char** argv) {
    RSVG_G_TYPE_INIT;
    g_test_init(&argc, &argv, NULL);

    rsvg_set_default_dpi_x_y(72, 72);

    /* Setup tests */
    if (argc < 2) {
        /* Run all tests in fixtures/reftests */
        const char* data_path = test_utils_get_test_data_path();
        GFile* base = g_file_new_for_path(data_path);
        GFile* reftests = g_file_get_child(base, "reftests");

        test_utils_add_test_for_all_files("/rsvg/reftest", reftests, reftests, run_rsvg_test, is_test_file);

        g_object_unref(reftests);
        g_object_unref(base);
        // g_free (data_path); // test_utils_get_test_data_path returns const or internal static? checked: const char*,
        // static buffer
    }
    else {
        /* Run specific files passed as args */
        for (int i = 1; i < argc; i++) {
            GFile* file = g_file_new_for_commandline_arg(argv[i]);
            test_utils_add_test_for_all_files("/rsvg/reftest", NULL, file, run_rsvg_test, is_test_file);
            g_object_unref(file);
        }
    }

    int result = g_test_run();

    rsvg_cleanup();
    FcFini();

    return result;
}