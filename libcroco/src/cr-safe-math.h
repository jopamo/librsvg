/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
#ifndef CR_SAFE_MATH_H
#define CR_SAFE_MATH_H

#include <glib.h>

G_BEGIN_DECLS

static inline gboolean cr_safe_size_mul(gsize a, gsize b, gsize* out) {
    if (a == 0 || b == 0) {
        *out = 0;
        return TRUE;
    }
    if (a > G_MAXSIZE / b) {
        return FALSE;
    }
    *out = a * b;
    return TRUE;
}

static inline gboolean cr_safe_size_add(gsize a, gsize b, gsize* out) {
    if (a > G_MAXSIZE - b) {
        return FALSE;
    }
    *out = a + b;
    return TRUE;
}

G_END_DECLS

#endif /* CR_SAFE_MATH_H */
