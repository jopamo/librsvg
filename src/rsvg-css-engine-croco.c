/*
   rsvg-css-engine-croco.c: Libcroco CSS Engine Backend

   Copyright (C) 2026 ...
*/

#include "config.h"
#include "rsvg-css-engine.h"
#include "rsvg-styles.h"
#include "rsvg-css.h"
#include "rsvg-private.h"

#include <string.h>
#include <libcroco.h>

typedef struct {
    RsvgCssEngine parent;
    GHashTable* css_props;
    RsvgHandle* ctx; /* Reference to handle for data acquisition */
} RsvgCssEngineCroco;

static void rsvg_css_engine_croco_free(RsvgCssEngine* engine) {
    RsvgCssEngineCroco* self = (RsvgCssEngineCroco*)engine;
    if (self->css_props) {
        g_hash_table_destroy(self->css_props);
    }
    g_free(self);
}

/* --- Libcroco parsing callbacks (moved from rsvg-css.c) --- */

#define RSVG_MAX_CSS_SIZE (1024 * 1024) /* 1MB */
#define RSVG_MAX_CSS_RULES 5000
#define RSVG_MAX_CSS_DECLARATIONS 50000
#define RSVG_MAX_CSS_SELECTOR_LENGTH 512

static void rsvg_css_define_style(RsvgCssEngineCroco* self,
                                  const gchar* selector,
                                  const gchar* style_name,
                                  const gchar* style_value,
                                  gboolean important) {
    GHashTable* styles;
    gboolean need_insert = FALSE;

    /* push name/style pair into HT */
    styles = g_hash_table_lookup(self->css_props, selector);
    if (styles == NULL) {
        styles = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)rsvg_style_value_data_free);
        g_hash_table_insert(self->css_props, (gpointer)g_strdup(selector), styles);
        need_insert = TRUE;
    }
    else {
        StyleValueData* current_value;
        current_value = g_hash_table_lookup(styles, style_name);
        if (current_value == NULL || !current_value->important)
            need_insert = TRUE;
    }
    if (need_insert) {
        g_hash_table_insert(styles, (gpointer)g_strdup(style_name),
                            (gpointer)rsvg_style_value_data_new(style_value, important));
    }
}

typedef struct _CSSUserData {
    RsvgCssEngineCroco* engine;
    CRSelector* selector;
    gsize num_rules;
    gsize num_declarations;
} CSSUserData;

static void css_user_data_init(CSSUserData* user_data, RsvgCssEngineCroco* engine) {
    user_data->engine = engine;
    user_data->selector = NULL;
    user_data->num_rules = 0;
    user_data->num_declarations = 0;
}

static void ccss_start_selector(CRDocHandler* a_handler, CRSelector* a_selector_list) {
    CSSUserData* user_data;

    g_return_if_fail(a_handler);

    user_data = (CSSUserData*)a_handler->app_data;

    if (user_data->num_rules >= RSVG_MAX_CSS_RULES) {
        return;
    }
    user_data->num_rules++;

    cr_selector_ref(a_selector_list);
    user_data->selector = a_selector_list;
}

static void ccss_end_selector(CRDocHandler* a_handler, CRSelector* a_selector_list) {
    CSSUserData* user_data;

    g_return_if_fail(a_handler);

    (void)a_selector_list;

    user_data = (CSSUserData*)a_handler->app_data;

    if (user_data->selector) {
        cr_selector_unref(user_data->selector);
        user_data->selector = NULL;
    }
}

static void ccss_property(CRDocHandler* a_handler, CRString* a_name, CRTerm* a_expr, gboolean a_important) {
    CSSUserData* user_data;
    gchar* name = NULL;
    size_t len = 0;

    g_return_if_fail(a_handler);

    user_data = (CSSUserData*)a_handler->app_data;

    if (user_data->num_declarations >= RSVG_MAX_CSS_DECLARATIONS) {
        return;
    }
    user_data->num_declarations++;

    if (a_name && a_expr && user_data->selector) {
        CRSelector* cur;
        for (cur = user_data->selector; cur; cur = cur->next) {
            if (cur->simple_sel) {
                gchar* selector = (gchar*)cr_simple_sel_to_string(cur->simple_sel);
                if (selector) {
                    if (strlen(selector) > RSVG_MAX_CSS_SELECTOR_LENGTH) {
                        g_free(selector);
                        continue;
                    }
                    gchar *style_name, *style_value;
                    name = (gchar*)cr_string_peek_raw_str(a_name);
                    len = cr_string_peek_raw_str_len(a_name);
                    style_name = g_strndup(name, len);
                    style_value = (gchar*)cr_term_to_string(a_expr);
                    rsvg_css_define_style(user_data->engine, selector, style_name, style_value, a_important);
                    g_free(selector);
                    g_free(style_name);
                    g_free(style_value);
                }
            }
        }
    }
}

static void ccss_error(CRDocHandler* a_handler) {
    (void)a_handler;
    g_warning(_("CSS parsing error\n"));
}

static void ccss_unrecoverable_error(CRDocHandler* a_handler) {
    (void)a_handler;
    g_warning(_("CSS unrecoverable error\n"));
}

static void ccss_import_style(CRDocHandler* a_this,
                              GList* a_media_list,
                              CRString* a_uri,
                              CRString* a_uri_default_ns,
                              CRParsingLocation* a_location);

static void init_sac_handler(CRDocHandler* a_handler) {
    a_handler->start_document = NULL;
    a_handler->end_document = NULL;
    a_handler->import_style = ccss_import_style;
    a_handler->namespace_declaration = NULL;
    a_handler->comment = NULL;
    a_handler->start_selector = ccss_start_selector;
    a_handler->end_selector = ccss_end_selector;
    a_handler->property = ccss_property;
    a_handler->start_font_face = NULL;
    a_handler->end_font_face = NULL;
    a_handler->start_media = NULL;
    a_handler->end_media = NULL;
    a_handler->start_page = NULL;
    a_handler->end_page = NULL;
    a_handler->ignorable_at_rule = NULL;
    a_handler->error = ccss_error;
    a_handler->unrecoverable_error = ccss_unrecoverable_error;
}

static gboolean rsvg_css_engine_croco_parse_stylesheet(RsvgCssEngine* engine, const char* buff, size_t buflen) {
    RsvgCssEngineCroco* self = (RsvgCssEngineCroco*)engine;
    CRParser* parser = NULL;
    CRDocHandler* css_handler = NULL;
    CSSUserData user_data;

    if (buff == NULL || buflen == 0)
        return TRUE;

    if (buflen > RSVG_MAX_CSS_SIZE) {
        g_warning(_("CSS buffer too large (%lu bytes), ignoring\n"), (unsigned long)buflen);
        return FALSE;
    }

    css_handler = cr_doc_handler_new();
    init_sac_handler(css_handler);

    css_user_data_init(&user_data, self);
    css_handler->app_data = &user_data;

    /* TODO: fix libcroco to take in const strings */
    parser = cr_parser_new_from_buf((guchar*)buff, (gulong)buflen, CR_UTF_8, FALSE);
    if (parser == NULL) {
        cr_doc_handler_unref(css_handler);
        return FALSE;
    }

    cr_parser_set_sac_handler(parser, css_handler);
    cr_doc_handler_unref(css_handler);

    cr_parser_set_use_core_grammar(parser, FALSE);
    cr_parser_parse(parser);

    cr_parser_destroy(parser);
    return TRUE;
}

static void ccss_import_style(CRDocHandler* a_this,
                              GList* a_media_list,
                              CRString* a_uri,
                              CRString* a_uri_default_ns,
                              CRParsingLocation* a_location) {
    CSSUserData* user_data = (CSSUserData*)a_this->app_data;
    char* stylesheet_data;
    gsize stylesheet_data_len;
    char* mime_type = NULL;

    (void)a_media_list;
    (void)a_uri_default_ns;
    (void)a_location;

    if (a_uri == NULL)
        return;

    /* Accessing ctx from engine */
    stylesheet_data = _rsvg_handle_acquire_data(user_data->engine->ctx, cr_string_peek_raw_str(a_uri), &mime_type,
                                                &stylesheet_data_len, NULL);
    if (stylesheet_data == NULL || mime_type == NULL || strcmp(mime_type, "text/css") != 0) {
        g_free(stylesheet_data);
        g_free(mime_type);
        return;
    }

    rsvg_css_engine_croco_parse_stylesheet((RsvgCssEngine*)user_data->engine, stylesheet_data, stylesheet_data_len);
    g_free(stylesheet_data);
    g_free(mime_type);
}

/* --- Style application logic (moved from rsvg-styles.c) --- */

typedef struct _StylesData {
    RsvgHandle* ctx;
    RsvgState* state;
} StylesData;

static void apply_style(const gchar* key, StyleValueData* value, gpointer user_data) {
    StylesData* data = (StylesData*)user_data;
    /* We need to expose rsvg_parse_style_pair or replicate it.
       It is static in rsvg-styles.c but declared as G_GNUC_INTERNAL in rsvg-styles.h?
       No, rsvg_parse_style_pairs is public (internal).
       rsvg_parse_style_pair is static.

       Wait, rsvg_parse_style calls rsvg_parse_style_pair.
       But rsvg_parse_style is for a string like "fill:red;...".
       Here we have key "fill" and value "red".

       I need to make rsvg_parse_style_pair internal public or use a wrapper.
       rsvg_styles.c has rsvg_parse_style_attrs which calls rsvg_lookup_apply_css_style which calls
       rsvg_parse_style_pair.

       I will need to expose rsvg_parse_style_pair from rsvg-styles.c
    */
    /* For now, assuming I will make it exposed as rsvg_parse_style_pair_internal */
    rsvg_parse_style_pair(data->ctx, data->state, key, value->value, value->important);
}

static gboolean rsvg_lookup_apply_css_style(RsvgCssEngineCroco* self,
                                            RsvgHandle* ctx,
                                            const char* target,
                                            RsvgState* state) {
    GHashTable* styles;

    styles = g_hash_table_lookup(self->css_props, target);

    if (styles != NULL) {
        StylesData* data = g_new(StylesData, 1);
        data->ctx = ctx;
        data->state = state;
        g_hash_table_foreach(styles, (GHFunc)apply_style, data);
        g_free(data);
        return TRUE;
    }
    return FALSE;
}

static void rsvg_css_engine_croco_apply_styles(RsvgCssEngine* engine,
                                               RsvgNode* node,
                                               RsvgState* state,
                                               const char* tag,
                                               const char* klass,
                                               const char* id,
                                               RsvgPropertyBag* atts) {
    RsvgCssEngineCroco* self = (RsvgCssEngineCroco*)engine;
    RsvgHandle* ctx = self->ctx;

    int i = 0, j = 0;
    char* target = NULL;
    gboolean found = FALSE;
    GString* klazz_list = NULL;

    if (atts != NULL && rsvg_property_bag_size(atts) > 0)
        rsvg_parse_style_pairs(ctx, state, atts);

    /* * */
    rsvg_lookup_apply_css_style(self, ctx, "*", state);

    /* tag */
    if (tag != NULL) {
        rsvg_lookup_apply_css_style(self, ctx, tag, state);
    }

    if (klass != NULL) {
        i = strlen(klass);
        while (j < i) {
            found = FALSE;
            klazz_list = g_string_new(".");

            while (j < i && g_ascii_isspace(klass[j]))
                j++;

            while (j < i && !g_ascii_isspace(klass[j]))
                g_string_append_c(klazz_list, klass[j++]);

            /* tag.class#id */
            if (tag != NULL && klazz_list->len != 1 && id != NULL) {
                target = g_strdup_printf("%s%s#%s", tag, klazz_list->str, id);
                found = found || rsvg_lookup_apply_css_style(self, ctx, target, state);
                g_free(target);
            }

            /* class#id */
            if (klazz_list->len != 1 && id != NULL) {
                target = g_strdup_printf("%s#%s", klazz_list->str, id);
                found = found || rsvg_lookup_apply_css_style(self, ctx, target, state);
                g_free(target);
            }

            /* tag.class */
            if (tag != NULL && klazz_list->len != 1) {
                target = g_strdup_printf("%s%s", tag, klazz_list->str);
                found = found || rsvg_lookup_apply_css_style(self, ctx, target, state);
                g_free(target);
            }

            /* didn't find anything more specific, just apply the class style */
            if (!found) {
                found = found || rsvg_lookup_apply_css_style(self, ctx, klazz_list->str, state);
            }
            g_string_free(klazz_list, TRUE);
        }
    }

    /* #id */
    if (id != NULL) {
        target = g_strdup_printf("#%s", id);
        rsvg_lookup_apply_css_style(self, ctx, target, state);
        g_free(target);
    }

    /* tag#id */
    if (tag != NULL && id != NULL) {
        target = g_strdup_printf("%s#%s", tag, id);
        rsvg_lookup_apply_css_style(self, ctx, target, state);
        g_free(target);
    }

    if (atts != NULL && rsvg_property_bag_size(atts) > 0) {
        const char* value;

        if ((value = rsvg_property_bag_lookup(atts, "style")) != NULL)
            rsvg_parse_style(ctx, state, value);
        if ((value = rsvg_property_bag_lookup(atts, "transform")) != NULL)
            rsvg_parse_transform_attr(ctx, state, value);
    }
    else if (node != NULL && node->style_attr != NULL) {
        rsvg_parse_style(ctx, state, node->style_attr);
    }
}

static const RsvgCssEngineVtable rsvg_css_engine_croco_vtable = {
    rsvg_css_engine_croco_free, rsvg_css_engine_croco_parse_stylesheet, rsvg_css_engine_croco_apply_styles};

RsvgCssEngine* rsvg_css_engine_croco_new(RsvgHandle* ctx) {
    RsvgCssEngineCroco* self = g_new0(RsvgCssEngineCroco, 1);
    self->parent.vtable = &rsvg_css_engine_croco_vtable;
    self->css_props = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_hash_table_destroy);
    self->ctx = ctx;
    return (RsvgCssEngine*)self;
}
