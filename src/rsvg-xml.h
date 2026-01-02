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

#ifndef RSVG_XML_H
#define RSVG_XML_H

#include <libxml/xmlIO.h>
#include <gio/gio.h>

G_BEGIN_DECLS

typedef struct _RsvgHandle RsvgHandle;

typedef struct {
    void (*start_element)(void* ctx, const xmlChar* name, const xmlChar** atts);
    void (*end_element)(void* ctx, const xmlChar* name);
    void (*characters)(void* ctx, const xmlChar* ch, int len);
    void (*processing_instruction)(void* ctx, const xmlChar* target, const xmlChar* data);
    xmlEntityPtr (*get_entity)(void* ctx, const xmlChar* name);
    void (*entity_decl)(void* ctx,
                        const xmlChar* name,
                        int type,
                        const xmlChar* publicId,
                        const xmlChar* systemId,
                        xmlChar* content);
    void (*unparsed_entity_decl)(void* ctx,
                                 const xmlChar* name,
                                 const xmlChar* publicId,
                                 const xmlChar* systemId,
                                 const xmlChar* notationName);
    xmlEntityPtr (*get_parameter_entity)(void* ctx, const xmlChar* name);
    void (*error)(void* ctx, const char* msg, ...);
    void (*cdata_block)(void* ctx, const xmlChar* value, int len);
} RsvgSaxCallbacks;

G_GNUC_INTERNAL
xmlParserCtxtPtr rsvg_create_xml_parser_from_stream(xmlSAXHandlerPtr sax,
                                                    void* sax_user_data,
                                                    GInputStream* stream,
                                                    GCancellable* cancellable,
                                                    GError** error);

G_GNUC_INTERNAL const RsvgSaxCallbacks* rsvg_get_sax_callbacks(void);

G_GNUC_INTERNAL xmlSAXHandlerPtr rsvg_xml_get_sax_handler(void);

G_GNUC_INTERNAL void rsvg_xml_init_default_sax_handler(xmlSAXHandler* handler);

G_GNUC_INTERNAL void rsvg_xml_configure_parser(xmlParserCtxtPtr parser, RsvgHandle* handle);

G_GNUC_INTERNAL void rsvg_xml_set_error(GError** error, xmlParserCtxtPtr ctxt);

G_END_DECLS

#endif /* !RSVG_XML_H */
