#include <stdio.h>
#include <stdlib.h>
#include "rsvg.h"

int main(int argc, char** argv) {
    GError* error = NULL;
    RsvgHandle* handle;

    printf("Starting reproduction...\n");

    handle = rsvg_handle_new_from_file("exploit.svg", &error);
    if (!handle) {
        fprintf(stderr, "Failed to load SVG: %s\n", error ? error->message : "unknown error");
        return 1;
    }

    printf("SVG loaded successfully.\n");

    // We don't strictly need to render to trigger the parsing of xi:include,
    // as it happens during the load phase (SAX parsing).

    g_object_unref(handle);
    return 0;
}
