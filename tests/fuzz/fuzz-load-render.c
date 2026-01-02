#include <stdint.h>
#include <stddef.h>
#include <glib.h>
#include "rsvg.h"
#include "rsvg-cairo.h"

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size);

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    RsvgHandle* handle;
    GError* error = NULL;

    handle = rsvg_handle_new_from_data(data, size, &error);
    if (handle) {
        RsvgDimensionData dimensions;
        rsvg_handle_get_dimensions(handle, &dimensions);

        /* Only render if it's reasonably small to avoid timeouts/OOM in fuzzing */
        if (dimensions.width > 0 && dimensions.width <= 1024 && dimensions.height > 0 && dimensions.height <= 1024) {
            cairo_surface_t* surface =
                cairo_image_surface_create(CAIRO_FORMAT_ARGB32, dimensions.width, dimensions.height);
            cairo_t* cr = cairo_create(surface);

            rsvg_handle_render_cairo(handle, cr);

            cairo_destroy(cr);
            cairo_surface_destroy(surface);
        }

        g_object_unref(handle);
    }
    if (error) {
        g_error_free(error);
    }

    return 0;
}

#ifndef RSVG_FUZZING_ENGINE
int main(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        g_autofree gchar* contents = NULL;
        gsize length = 0;
        if (g_file_get_contents(argv[i], &contents, &length, NULL)) {
            LLVMFuzzerTestOneInput((const uint8_t*)contents, length);
        }
    }
    return 0;
}
#endif
