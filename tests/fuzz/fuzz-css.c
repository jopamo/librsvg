#include <stdint.h>
#include <stddef.h>
#include <glib.h>
#include "rsvg.h"
#include "rsvg-css.h"

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size);

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    RsvgHandle* handle;

    handle = rsvg_handle_new();
    if (handle) {
        rsvg_parse_cssbuffer(handle, (const char*)data, size);
        g_object_unref(handle);
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
