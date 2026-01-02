/* -*- Mode: C; indent-tabs-mode:nil; c-basic-offset: 8-*- */

#include <glib.h>

#include "cr-additional-sel.h"
#include "cr-attr-sel.h"
#include "cr-fonts.h"
#include "cr-parsing-location.h"
#include "cr-pseudo.h"
#include "cr-rgb.h"
#include "cr-selector.h"
#include "cr-simple-sel.h"
#include "cr-statement.h"
#include "cr-string.h"
#include "cr-stylesheet.h"
#include "cr-term.h"

static CRTerm* make_function_term(const char* func_name, const char* param_ident) {
    CRTerm* param = cr_term_new();
    CRTerm* term = cr_term_new();

    g_assert_nonnull(param);
    g_assert_nonnull(term);

    g_assert_cmpint(cr_term_set_ident(param, cr_string_new_from_string(param_ident)), ==, CR_OK);
    g_assert_cmpint(cr_term_set_function(term, cr_string_new_from_string(func_name), param), ==, CR_OK);

    return term;
}

static void test_term_to_string_function(void) {
    CRTerm* term = make_function_term("foo", "bar");
    guchar* result = cr_term_to_string(term);

    g_assert_nonnull(result);
    g_assert_cmpstr((const gchar*)result, ==, "foo(bar)");

    g_free(result);
    cr_term_destroy(term);
}

static void test_term_one_to_string_function(void) {
    CRTerm* term = make_function_term("foo", "bar");
    guchar* result = cr_term_one_to_string(term);

    g_assert_nonnull(result);
    g_assert_cmpstr((const gchar*)result, ==, "foo(bar)");

    g_free(result);
    cr_term_destroy(term);
}

static void test_stylesheet_to_string(void) {
    const guchar* css = (const guchar*)"svg { fill: red; }";
    CRStatement* statement = cr_statement_parse_from_buf(css, CR_UTF_8);
    CRStyleSheet* sheet = cr_stylesheet_new(statement);
    gchar* result = NULL;

    g_assert_nonnull(statement);
    g_assert_nonnull(sheet);

    result = cr_stylesheet_to_string(sheet);
    g_assert_nonnull(result);
    g_assert_true(g_strstr_len(result, -1, "fill") != NULL);

    g_free(result);
    cr_stylesheet_destroy(sheet);
}

static void test_selector_to_string(void) {
    CRSimpleSel* simple = cr_simple_sel_new();
    CRSelector* selector = NULL;
    guchar* result = NULL;

    g_assert_nonnull(simple);

    simple->type_mask = TYPE_SELECTOR;
    simple->name = cr_string_new_from_string("svg");
    selector = cr_selector_new(simple);
    g_assert_nonnull(selector);

    result = cr_selector_to_string(selector);
    g_assert_nonnull(result);
    g_assert_true(g_strstr_len((const gchar*)result, -1, "svg") != NULL);

    g_free(result);
    cr_selector_destroy(selector);
}

static void test_rgb_to_string(void) {
    CRRgb* rgb = cr_rgb_new_with_vals(10, 20, 30, FALSE);
    guchar* result = NULL;

    g_assert_nonnull(rgb);

    result = cr_rgb_to_string(rgb);
    g_assert_nonnull(result);
    g_assert_true(g_strstr_len((const gchar*)result, -1, "10") != NULL);

    g_free(result);
    cr_rgb_destroy(rgb);
}

static void test_pseudo_to_string(void) {
    CRPseudo* pseudo = cr_pseudo_new();
    guchar* result = NULL;

    g_assert_nonnull(pseudo);

    pseudo->type = FUNCTION_PSEUDO;
    pseudo->name = cr_string_new_from_string("lang");
    pseudo->extra = cr_string_new_from_string("en");

    result = cr_pseudo_to_string(pseudo);
    g_assert_nonnull(result);
    g_assert_true(g_strstr_len((const gchar*)result, -1, "lang") != NULL);

    g_free(result);
    cr_pseudo_destroy(pseudo);
}

static void test_attr_sel_to_string(void) {
    CRAttrSel* attr = cr_attr_sel_new();
    guchar* result = NULL;

    g_assert_nonnull(attr);

    attr->name = cr_string_new_from_string("href");
    attr->value = cr_string_new_from_string("example");
    attr->match_way = EQUALS;

    result = cr_attr_sel_to_string(attr);
    g_assert_nonnull(result);
    g_assert_true(g_strstr_len((const gchar*)result, -1, "href") != NULL);

    g_free(result);
    cr_attr_sel_destroy(attr);
}

static void test_additional_sel_to_string(void) {
    CRAdditionalSel* additional = cr_additional_sel_new_with_type(CLASS_ADD_SELECTOR);
    guchar* result = NULL;

    g_assert_nonnull(additional);

    cr_additional_sel_set_class_name(additional, cr_string_new_from_string("primary"));

    result = cr_additional_sel_one_to_string(additional);
    g_assert_nonnull(result);
    g_assert_true(g_strstr_len((const gchar*)result, -1, ".primary") != NULL);

    g_free(result);
    cr_additional_sel_destroy(additional);
}

static void test_font_family_to_string(void) {
    CRFontFamily* family = cr_font_family_new(FONT_FAMILY_NON_GENERIC, (guchar*)g_strdup("Sans"));
    guchar* result = NULL;

    g_assert_nonnull(family);

    result = cr_font_family_to_string(family, FALSE);
    g_assert_nonnull(result);
    g_assert_true(g_strstr_len((const gchar*)result, -1, "Sans") != NULL);

    g_free(result);
    cr_font_family_destroy(family);
}

static void test_parsing_location_to_string(void) {
    CRParsingLocation location;
    gchar* result = NULL;

    location.line = 1;
    location.column = 2;
    location.byte_offset = 3;

    result = cr_parsing_location_to_string(&location, DUMP_LINE | DUMP_COLUMN | DUMP_BYTE_OFFSET);
    g_assert_nonnull(result);
    g_assert_true(g_strstr_len(result, -1, "line:1") != NULL);

    g_free(result);
}

static void test_statement_list_to_string(void) {
    const guchar* statement_text = (const guchar*)"svg { fill: red; }";
    CRStatement* statement = cr_statement_parse_from_buf(statement_text, CR_UTF_8);
    gchar* result = NULL;

    g_assert_nonnull(statement);

    result = cr_statement_list_to_string(statement, 0);
    g_assert_nonnull(result);
    g_assert_true(g_strstr_len(result, -1, "fill") != NULL);

    g_free(result);
    cr_statement_destroy(statement);
}

int main(int argc, char** argv) {
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/libcroco/term/to-string", test_term_to_string_function);
    g_test_add_func("/libcroco/term/one-to-string", test_term_one_to_string_function);
    g_test_add_func("/libcroco/stylesheet/to-string", test_stylesheet_to_string);
    g_test_add_func("/libcroco/selector/to-string", test_selector_to_string);
    g_test_add_func("/libcroco/rgb/to-string", test_rgb_to_string);
    g_test_add_func("/libcroco/pseudo/to-string", test_pseudo_to_string);
    g_test_add_func("/libcroco/attr-sel/to-string", test_attr_sel_to_string);
    g_test_add_func("/libcroco/additional-sel/to-string", test_additional_sel_to_string);
    g_test_add_func("/libcroco/font-family/to-string", test_font_family_to_string);
    g_test_add_func("/libcroco/parsing-location/to-string", test_parsing_location_to_string);
    g_test_add_func("/libcroco/statement/list-to-string", test_statement_list_to_string);

    return g_test_run();
}
