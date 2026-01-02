/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim: set sw=4 sts=4 ts=4 expandtab: */

#include "config.h"

#include <glib.h>

static void test_ci_enabled_define(void) {
    g_assert_cmpint(RSVG_CI, ==, 1);
}

int main(int argc, char** argv) {
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/build/ci-option/enabled", test_ci_enabled_define);

    return g_test_run();
}
