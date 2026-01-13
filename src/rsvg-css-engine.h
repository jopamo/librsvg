/*
   rsvg-css-engine.h: CSS Engine Abstraction

   Copyright (C) 2026 ...
*/

#ifndef RSVG_CSS_ENGINE_H
#define RSVG_CSS_ENGINE_H

#include <glib.h>
#include <glib-object.h>
#include "rsvg-private.h"

G_BEGIN_DECLS

typedef struct _RsvgCssEngineVtable RsvgCssEngineVtable;

struct _RsvgCssEngineVtable {
    void (*free)(RsvgCssEngine* engine);
    gboolean (*parse_stylesheet)(RsvgCssEngine* engine, const char* data, size_t len);
    void (*apply_styles)(RsvgCssEngine* engine,
                         RsvgNode* node,
                         RsvgState* state,
                         const char* tag,
                         const char* klass,
                         const char* id,
                         RsvgPropertyBag* atts);
};

struct _RsvgCssEngine {
    const RsvgCssEngineVtable* vtable;
};

G_GNUC_INTERNAL
void rsvg_css_engine_free(RsvgCssEngine* engine);

G_GNUC_INTERNAL
gboolean rsvg_css_engine_parse_stylesheet(RsvgCssEngine* engine, const char* data, size_t len);

G_GNUC_INTERNAL
void rsvg_css_engine_apply_styles(RsvgCssEngine* engine,
                                  RsvgNode* node,
                                  RsvgState* state,
                                  const char* tag,
                                  const char* klass,
                                  const char* id,
                                  RsvgPropertyBag* atts);

/* Factory for default (Libcroco) engine */
G_GNUC_INTERNAL
RsvgCssEngine* rsvg_css_engine_croco_new(RsvgHandle* ctx);

G_END_DECLS

#endif /* RSVG_CSS_ENGINE_H */
