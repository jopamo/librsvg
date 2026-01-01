/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim: set sw=4 sts=4 ts=4 expandtab: */

#ifndef RSVG_DEPRECATED_H
#define RSVG_DEPRECATED_H

#if !defined(__RSVG_RSVG_H_INSIDE__) && !defined(RSVG_COMPILATION)
#warning "Including <librsvg/rsvg-deprecated.h> directly is deprecated."
#endif

G_BEGIN_DECLS

/* BEGIN deprecated APIs. Do not use! */

#ifndef __GI_SCANNER__

RSVG_DEPRECATED_FOR(g_type_init)
void rsvg_init(void);
RSVG_DEPRECATED
void rsvg_term(void);

RSVG_DEPRECATED_FOR(g_object_unref)
void rsvg_handle_free(RsvgHandle* handle);

/**
 * RsvgSizeFunc:
 * @width: (out): the width of the SVG
 * @height: (out): the height of the SVG
 * @user_data: user data
 *
 * Function to let a user of the library specify the SVG's dimensions
 *
 * Deprecated: Set up a cairo matrix and use rsvg_handle_render_cairo() instead.
 * See the documentation for rsvg_handle_set_size_callback() for an example.
 */
typedef /* RSVG_DEPRECATED */ void (*RsvgSizeFunc)(gint* width, gint* height, gpointer user_data);

RSVG_DEPRECATED
void rsvg_handle_set_size_callback(RsvgHandle* handle,
                                   RsvgSizeFunc size_func,
                                   gpointer user_data,
                                   GDestroyNotify user_data_destroy);

/* GdkPixbuf convenience API */

RSVG_DEPRECATED
GdkPixbuf* rsvg_pixbuf_from_file(const gchar* file_name, GError** error);
RSVG_DEPRECATED
GdkPixbuf* rsvg_pixbuf_from_file_at_zoom(const gchar* file_name, double x_zoom, double y_zoom, GError** error);
RSVG_DEPRECATED
GdkPixbuf* rsvg_pixbuf_from_file_at_size(const gchar* file_name, gint width, gint height, GError** error);
RSVG_DEPRECATED
GdkPixbuf* rsvg_pixbuf_from_file_at_max_size(const gchar* file_name, gint max_width, gint max_height, GError** error);
RSVG_DEPRECATED
GdkPixbuf* rsvg_pixbuf_from_file_at_zoom_with_max(const gchar* file_name,
                                                  double x_zoom,
                                                  double y_zoom,
                                                  gint max_width,
                                                  gint max_height,
                                                  GError** error);

RSVG_DEPRECATED
const char* rsvg_handle_get_title(RsvgHandle* handle);
RSVG_DEPRECATED
const char* rsvg_handle_get_desc(RsvgHandle* handle);
RSVG_DEPRECATED
const char* rsvg_handle_get_metadata(RsvgHandle* handle);

#endif /* !__GI_SCANNER__ */

/* END deprecated APIs. */

G_END_DECLS

#endif /* RSVG_DEPRECATED_H */
