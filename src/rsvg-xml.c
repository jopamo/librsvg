/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim: set sw=4 sts=4 expandtab: */
/*
 * Copyright © 2010 Christian Persch
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
*/

#include "config.h"

#include "rsvg-private.h"
#include "rsvg-xml.h"

#include <string.h>

static xmlSAXHandler rsvg_sax_handler;
static gboolean rsvg_sax_handler_inited = FALSE;

static void rsvg_xml_init_limits(void) {
    static gboolean limits_inited = FALSE;
    if (limits_inited)
        return;
    limits_inited = TRUE;

#ifdef XML_PARSE_LIMIT_DEPTH
    xmlParserMaxDepth = 256;
#endif
#ifdef XML_PARSE_LIMIT_ENTITY_EXPANSION
    xmlParserMaxEntityExpansions = 100000;
#endif
#ifdef XML_PARSE_LIMIT_ENTITY_SIZE
    xmlParserMaxEntitySize = 10 * 1024 * 1024; /* 10 MB */
#endif
}

typedef struct {
    GInputStream* stream;
    GCancellable* cancellable;
    GError** error;
} RsvgXmlInputStreamContext;

static void rsvg_xml_start_element_sax1(void* ctx, const xmlChar* name, const xmlChar** atts) {
    const RsvgSaxCallbacks* callbacks = rsvg_get_sax_callbacks();

    if (!callbacks || !callbacks->start_element)
        return;

    callbacks->start_element(ctx, name, atts);
}

static void rsvg_xml_end_element_sax1(void* ctx, const xmlChar* name) {
    const RsvgSaxCallbacks* callbacks = rsvg_get_sax_callbacks();

    if (!callbacks || !callbacks->end_element)
        return;

    callbacks->end_element(ctx, name);
}

static void rsvg_xml_init_sax_handler(void) {
    const RsvgSaxCallbacks* callbacks;

    rsvg_xml_init_limits();

    if (rsvg_sax_handler_inited)
        return;

    rsvg_sax_handler_inited = TRUE;

    memset(&rsvg_sax_handler, 0, sizeof(rsvg_sax_handler));
    rsvg_sax_handler.initialized = XML_SAX2_MAGIC;

    callbacks = rsvg_get_sax_callbacks();
    if (!callbacks)
        return;

    rsvg_sax_handler.getEntity = callbacks->get_entity;
    rsvg_sax_handler.entityDecl = callbacks->entity_decl;
    rsvg_sax_handler.unparsedEntityDecl = callbacks->unparsed_entity_decl;
    rsvg_sax_handler.getParameterEntity = callbacks->get_parameter_entity;
    rsvg_sax_handler.characters = callbacks->characters;
    rsvg_sax_handler.ignorableWhitespace = callbacks->characters;
    rsvg_sax_handler.cdataBlock = callbacks->cdata_block;
    rsvg_sax_handler.startElement = rsvg_xml_start_element_sax1;
    rsvg_sax_handler.endElement = rsvg_xml_end_element_sax1;
    rsvg_sax_handler.startElementNs = NULL;
    rsvg_sax_handler.endElementNs = NULL;
    rsvg_sax_handler.processingInstruction = callbacks->processing_instruction;
    rsvg_sax_handler.error = callbacks->error;
}

xmlSAXHandlerPtr rsvg_xml_get_sax_handler(void) {
    rsvg_xml_init_sax_handler();
    return &rsvg_sax_handler;
}

void rsvg_xml_init_default_sax_handler(xmlSAXHandler* handler) {
    memset(handler, 0, sizeof(*handler));
    xmlSAX2InitDefaultSAXHandler(handler, 0);
}

void rsvg_xml_configure_parser(xmlParserCtxtPtr parser, RsvgHandle* handle) {
    int options;

    if (!parser)
        return;

    options = (XML_PARSE_NONET | XML_PARSE_BIG_LINES | XML_PARSE_NOENT);

    if (handle && (handle->priv->flags & RSVG_HANDLE_FLAG_UNLIMITED)) {
        options |= XML_PARSE_HUGE;
    }
    else {
#ifdef XML_PARSE_LIMIT_DEPTH
        options |= XML_PARSE_LIMIT_DEPTH;
#endif
#ifdef XML_PARSE_LIMIT_ENTITY_EXPANSION
        options |= XML_PARSE_LIMIT_ENTITY_EXPANSION;
#endif
#ifdef XML_PARSE_LIMIT_ENTITY_SIZE
        options |= XML_PARSE_LIMIT_ENTITY_SIZE;
#endif
    }

#ifdef XML_PARSE_NO_XXE
    options |= XML_PARSE_NO_XXE;
#endif

    xmlCtxtUseOptions(parser, options);
}

void rsvg_xml_set_error(GError** error, xmlParserCtxtPtr ctxt) {
    (void)ctxt;
    g_set_error_literal(error, rsvg_error_quark(), RSVG_ERROR_FAILED, _("Error parsing XML data"));
}

/* this should use gsize, but libxml2 is borked */
static int context_read(void* data, char* buffer, int len) {
    RsvgXmlInputStreamContext* context = data;
    gssize n_read;

    if (*(context->error))
        return -1;

    n_read = g_input_stream_read(context->stream, buffer, (gsize)len, context->cancellable, context->error);
    if (n_read < 0)
        return -1;

    return (int)n_read;
}

static int context_close(void* data) {
    RsvgXmlInputStreamContext* context = data;
    gboolean ret;

    /* Don't overwrite a previous error */
    ret =
        g_input_stream_close(context->stream, context->cancellable, *(context->error) == NULL ? context->error : NULL);

    g_object_unref(context->stream);
    if (context->cancellable)
        g_object_unref(context->cancellable);
    g_slice_free(RsvgXmlInputStreamContext, context);

    return ret ? 0 : -1;
}

xmlParserCtxtPtr rsvg_create_xml_parser_from_stream(xmlSAXHandlerPtr sax,
                                                    void* sax_user_data,
                                                    GInputStream* stream,
                                                    GCancellable* cancellable,
                                                    GError** error) {
    RsvgXmlInputStreamContext* context;
    xmlParserCtxtPtr parser;

    g_return_val_if_fail(G_IS_INPUT_STREAM(stream), NULL);
    g_return_val_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable), NULL);
    g_return_val_if_fail(error != NULL, NULL);

    context = g_slice_new(RsvgXmlInputStreamContext);
    context->stream = g_object_ref(stream);
    context->cancellable = cancellable ? g_object_ref(cancellable) : NULL;
    context->error = error;

    parser = xmlCreateIOParserCtxt(sax, sax_user_data, context_read, context_close, context, XML_CHAR_ENCODING_NONE);

    if (!parser) {
        g_set_error(error, rsvg_error_quark(), 0, _("Error creating XML parser"));

        /* on error, xmlCreateIOParserCtxt() frees our context via the context_close function */
    }

    return parser;
}

void rsvg_SAX_handler_struct_init(void) {
    rsvg_xml_get_sax_handler();
}
