# Current CSS Support in librsvg

This document inventories the CSS support in `librsvg` as of the `libcroco` baseline. It is derived from code analysis of `src/rsvg-styles.c` and `src/rsvg-css.c`.

## Supported Properties

The following properties are explicitly parsed and handled in `rsvg_parse_style_pairs`:

*   `baseline-shift`
*   `clip-path`
*   `clip-rule`
*   `color`
*   `comp-op` (Custom: Cairo operator)
*   `direction`
*   `display`
*   `enable-background`
*   `fill`
*   `fill-opacity`
*   `fill-rule`
*   `filter`
*   `flood-color`
*   `flood-opacity`
*   `font-family`
*   `font-size`
*   `font-stretch`
*   `font-style`
*   `font-variant`
*   `font-weight`
*   `letter-spacing`
*   `marker-end`
*   `marker-mid`
*   `marker-start`
*   `mask`
*   `opacity`
*   `overflow`
*   `shape-rendering`
*   `stop-color`
*   `stop-opacity`
*   `stroke`
*   `stroke-dasharray`
*   `stroke-dashoffset`
*   `stroke-linecap`
*   `stroke-linejoin`
*   `stroke-miterlimit`
*   `stroke-opacity`
*   `stroke-width`
*   `text-anchor`
*   `text-decoration`
*   `text-rendering` (Found in `rsvg_parse_style_pair` but missed in initial `rsvg_parse_style_pairs` list check - verified in `rsvg_parse_style_pair` body)
*   `unicode-bidi`
*   `visibility`
*   `writing-mode`
*   `xml:lang`
*   `xml:space`

## Supported Selectors

`librsvg` currently implements a strictly limited subset of CSS Level 2 selectors. The limitation stems from the lookup strategy in `rsvg_parse_style_attrs`, which constructs specific key strings based on the current node's attributes and looks them up in a hash table.

**Supported patterns:**

*   Universal: `*`
*   Type: `tag`
*   Class: `.class`
*   ID: `#id`
*   Type + Class: `tag.class`
*   Type + ID: `tag#id`
*   Class + ID: `.class#id`
*   Type + Class + ID: `tag.class#id`

**Unsupported:**

*   **Descendant combinators** (e.g., `div p`) - Likely flattened to the simple selector or ignored.
*   **Child combinators** (e.g., `div > p`)
*   **Sibling combinators** (e.g., `h1 + p`)
*   **Pseudo-classes** (e.g., `:first-child`, `:hover`) - No logic exists to generate lookup keys involving pseudo-classes.
*   **Attribute selectors** (e.g., `[type="text"]`) - Not constructed in the lookup keys.

## Parsing Quirks & Limitations

1.  **String Round-tripping:**
    *   CSS is parsed by `libcroco`, converted *back* to a string via `cr_term_to_string`, stored in a hash table, and then re-parsed by `librsvg`'s ad-hoc parsers when applied.
    *   This means `libcroco`'s understanding of the value must survive serialization to string and `librsvg`'s re-parsing.

2.  **Hard Constraints:**
    *   Max CSS buffer size: 1MB (`RSVG_MAX_CSS_SIZE`)
    *   Max CSS rules: 5000 (`RSVG_MAX_CSS_RULES`)
    *   Max CSS declarations: 50000 (`RSVG_MAX_CSS_DECLARATIONS`)
    *   Max Selector Length: 512 bytes (`RSVG_MAX_CSS_SELECTOR_LENGTH`)

3.  **Ad-hoc Parsing:**
    *   `font-family` splitting is done via `strtok` on commas, which may break font names containing commas (though rare).
    *   Single quotes are stripped "in a trivial way" in `rsvg_parse_style`.
    *   `!important` is handled by splitting strings on `!`.

4.  **Error Handling:**
    *   CSS parsing errors trigger a `g_warning` but do not stop processing.
