/*
   rsvg-css-engine.c: CSS Engine Abstraction

   Copyright (C) 2026 ...
*/

#include "config.h"
#include "rsvg-css-engine.h"

void rsvg_css_engine_free(RsvgCssEngine* engine) {
    if (engine && engine->vtable && engine->vtable->free) {
        engine->vtable->free(engine);
    }
}

gboolean rsvg_css_engine_parse_stylesheet(RsvgCssEngine* engine, const char* data, size_t len) {
    if (engine && engine->vtable && engine->vtable->parse_stylesheet) {
        return engine->vtable->parse_stylesheet(engine, data, len);
    }
    return FALSE;
}

void rsvg_css_engine_apply_styles(RsvgCssEngine* engine,
                                  RsvgNode* node,
                                  RsvgState* state,
                                  const char* tag,
                                  const char* klass,
                                  const char* id,
                                  RsvgPropertyBag* atts) {
    if (engine && engine->vtable && engine->vtable->apply_styles) {
        engine->vtable->apply_styles(engine, node, state, tag, klass, id, atts);
    }
}
