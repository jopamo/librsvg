#include <stdint.h>
#include <stddef.h>
#include <glib.h>
#include <cairo.h>
#include "rsvg-path.h"

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size);

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    g_autofree char* path_str = g_strndup((const char*)data, size);
    cairo_path_t* path;

    path = rsvg_parse_path(path_str);
    if (path) {
        rsvg_cairo_path_destroy(path);
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
