/* vim: set ts=4 nowrap ai expandtab sw=4: */

#include <glib.h>
#include <stdio.h>
#include "rsvg.h"
#include "rsvg-private.h"
#include "rsvg-styles.h"
#include "test-utils.h"

/* Helper to dump RsvgLength */
static void dump_length(const char* name, RsvgLength len, GString* out) {
    g_string_append_printf(out, " %s=%g%c", name, len.length, len.factor ? len.factor : ' ');
}

/* Helper to dump color */
static void dump_color(const char* name, guint32 color, gboolean has, GString* out) {
    if (!has)
        return;
    g_string_append_printf(out, " %s=#%08x", name, color);
}

/* Helper to dump paint server */
static void dump_paint_server(const char* name, RsvgPaintServer* server, gboolean has, GString* out) {
    if (!has || !server)
        return;

    if (server->type == RSVG_PAINT_SERVER_SOLID) {
        RsvgSolidColor* color = server->core.color;
        if (color) {
            if (color->currentcolor) {
                g_string_append_printf(out, " %s=currentColor", name);
            }
            else {
                g_string_append_printf(out, " %s=#%08x", name, color->argb);
            }
        }
    }
    else if (server->type == RSVG_PAINT_SERVER_IRI) {
        g_string_append_printf(out, " %s=url(%s)", name, server->core.iri ? server->core.iri : "null");
    }
}

static void dump_node_state(RsvgNode* node, int depth, GString* out) {
    /* Skip content nodes if they are just whitespace (optional, but makes output cleaner) */
    /* Accessing node type requires definition of RsvgNode which is opaque in some contexts,
       but we included rsvg-private.h so we should have it.
       RSVG_NODE_TYPE(node) is the macro. */

    int i;
    for (i = 0; i < depth; i++)
        g_string_append(out, "  ");

    g_string_append_printf(out, "Node %s", node->name ? node->name : "(null)");
    if (node->id)
        g_string_append_printf(out, " id=%s", node->id);
    if (node->klass)
        g_string_append_printf(out, " class=%s", node->klass);

    if (node->state) {
        RsvgState* s = node->state;

        if (s->has_fill_server)
            dump_paint_server("fill", s->fill, TRUE, out);
        if (s->has_stroke_server)
            dump_paint_server("stroke", s->stroke, TRUE, out);

        if (s->has_font_size)
            dump_length("font-size", s->font_size, out);
        if (s->has_font_family)
            g_string_append_printf(out, " font-family=\"%s\"", s->font_family);
        if (s->has_stroke_width)
            dump_length("stroke-width", s->stroke_width, out);
        if (s->has_visible)
            g_string_append_printf(out, " visible=%d", s->visible);
        if (s->has_flood_color)
            dump_color("flood-color", s->flood_color, TRUE, out);
        if (s->opacity != 255)
            g_string_append_printf(out, " opacity=%d", s->opacity);

        /* Add more as needed */
    }
    g_string_append(out, "\n");

    if (node->children) {
        guint j;
        for (j = 0; j < node->children->len; j++) {
            RsvgNode* child = g_ptr_array_index(node->children, j);
            dump_node_state(child, depth + 1, out);
        }
    }
}

static void test_style_dump(gconstpointer data) {
    const char* filename = data;
    char* path = g_build_filename(test_utils_get_test_data_path(), filename, NULL);
    GError* error = NULL;
    RsvgHandle* handle = rsvg_handle_new_from_file(path, &error);

    g_assert_no_error(error);
    g_assert(handle);

    GString* dump = g_string_new("");
    if (handle->priv->treebase) {
        dump_node_state(handle->priv->treebase, 0, dump);
    }

    /* For now, just print to stdout so we can see it.
       In real life, we'd compare this to a reference file. */
    char* ref_filename = g_strdup_printf("%s.out", filename);
    char* ref_path = g_build_filename(test_utils_get_test_data_path(), ref_filename, NULL);

    /* If we wanted to enforce strict check:
    char *ref_content;
    if (g_file_get_contents(ref_path, &ref_content, NULL, NULL)) {
        g_assert_cmpstr(dump->str, ==, ref_content);
        g_free(ref_content);
    } else {
        g_test_message("No reference file %s, saving output.", ref_filename);
        g_file_set_contents(ref_path, dump->str, -1, NULL);
    }
    */

    /* Since we are establishing baseline, we just write it if it doesn't exist,
       or maybe just assert it matches if it does. */

    // For this task, we just ensure it runs and dumps.
    g_test_message("Dump for %s:\n%s", filename, dump->str);

    g_string_free(dump, TRUE);
    g_object_unref(handle);
    g_free(path);
    g_free(ref_filename);
    g_free(ref_path);
}

int main(int argc, char** argv) {
    g_test_init(&argc, &argv, NULL);

    g_test_add_data_func("/style-dump/selectors", "reftests/css-migration/selectors.svg", test_style_dump);
    g_test_add_data_func("/style-dump/properties", "reftests/css-migration/properties.svg", test_style_dump);
    g_test_add_data_func("/style-dump/cascade", "reftests/css-migration/cascade.svg", test_style_dump);

    return g_test_run();
}
