The following analysis and changes were made to address potential vulnerabilities related to browser-context script execution and external resource loading in `librsvg`.

### Vulnerability Analysis

1.  **Script Execution (XSS):**
    *   `librsvg` does not link against any JavaScript engine, so it cannot execute `<script>` tags or event handlers directly.
    *   `<script>` tags are treated as unknown elements (handled as `defs`), meaning they are not rendered but are preserved in the internal node tree.
    *   **Vulnerability Found:** When retrieving metadata via `rsvg_handle_get_metadata()`, the library reconstructs the XML for the metadata element. The previous implementation failed to escape attribute values, allowing for **XML Injection / Stored XSS**. A crafted SVG could inject arbitrary attributes (e.g., `onload="alert(1)"`) into the metadata output, which, if embedded by an application in a web context, would lead to XSS.

2.  **External Resource Loading (SSRF / LFI):**
    *   Resource loading (images, CSS imports, XML entities) is handled by `_rsvg_handle_acquire_data` and `rsvg_allow_load`.
    *   **Mitigation:** `rsvg_allow_load` implements a strict policy:
        *   It denies loading resources with a scheme different from the base file (e.g., local files cannot load `http` resources).
        *   For local files, it enforces that the target resource must be within the same directory tree as the base SVG file (preventing traversal like `../../etc/passwd`).
    *   This effectively mitigates indiscriminate Local File Inclusion (LFI) and Server-Side Request Forgery (SSRF) via `xlink:href` or entities.

3.  **XXE (XML External Entities):**
    *   The XML parser is configured with `XML_PARSE_NONET`.
    *   Entity loading is intercepted and routed through the same `_rsvg_handle_acquire_data` function, subjecting it to the strict path checks mentioned above.

### Fix Applied

I applied a fix to `src/rsvg-base.c` in the `rsvg_metadata_props_enumerate` function.

**Change:**
The code now properly escapes the attribute values using `g_markup_escape_text` before appending them to the metadata string.

```c
static void rsvg_metadata_props_enumerate(const char* key, const char* value, gpointer user_data) {
    GString* metadata = (GString*)user_data;
    // Fix: Escape the value to prevent XML injection
    char* escaped = g_markup_escape_text(value, -1);
    g_string_append_printf(metadata, "%s=\"%s\" ", key, escaped);
    g_free(escaped);
}
```

This prevents the injection of malicious attributes or malformed XML in the metadata output.
