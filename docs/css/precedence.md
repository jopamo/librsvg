# CSS Precedence Rules in librsvg

This document describes the *actual* precedence rules implemented in `librsvg`'s current `libcroco`-based engine. These rules differ from standard CSS cascade behavior due to the architectural limitations of the implementation.

## Application Order (Specificity Emulation)

`librsvg` does not calculate specificity scores. Instead, it applies styles in a fixed sequence. Since later applications overwrite earlier ones (unless blocked by `!important`), the application order effectively defines the precedence.

The order of application for a given node is:

1.  **Presentation Attributes:** (e.g., `fill="red"`)
    *   Applied first.
    *   Lowest priority.

2.  **Universal Selector:** `*`

3.  **Type Selector:** `tagname` (e.g., `rect`, `path`)

4.  **Class Selectors:**
    *   For each class in the `class` attribute:
        1.  `tagname.classname#id`
        2.  `.classname#id`
        3.  `tagname.classname`
        4.  `.classname`
    *   *Note:* This loop implies that if an element has multiple classes, the styles for the *last* class in the attribute list will override the earlier ones for conflicting properties, assuming equal "structural" specificity (e.g. both are just `.classname`).

5.  **ID Selector:** `#id`

6.  **Type + ID Selector:** `tagname#id`

7.  **Inline Style:** `style="..."` attribute.
    *   Applied last.
    *   Highest priority among author origins.

## `!important` Handling

*   The `!important` flag is preserved when parsing CSS rules.
*   When a property is applied, `librsvg` checks if the *existing* value for that property is marked `!important`.
*   If the existing value is `!important` and the new value is *not*, the new value is ignored.
*   This mechanism works correctly across the application steps above. For example, a presentation attribute with `!important` (if such a thing existed, though invalid SVG) would block a normal rule in the `#id` step.

## Divergences from Standard CSS

1.  **No True Specificity:**
    *   A class selector `.myclass` always loses to `#myid`, regardless of file order or how many classes are chained.
    *   `tag#id` beats `#id`, which is non-standard (usually ID is the dominant factor).
    *   The "specificity" is hardcoded into the C function `rsvg_parse_style_attrs`.

2.  **Order of Appearance Ignored (mostly):**
    *   Within the same specificity bucket (e.g., two `.class` rules), `librsvg` relies on the hash table storage.
    *   `rsvg_css_define_style` overwrites entries in the hash table.
    *   This means if two rules match the *exact same selector string* (e.g. `rect { ... }` and later `rect { ... }`), the last one parsed wins (correct).
    *   However, if two *different* selectors apply to the same element but fall into the same step above (e.g. `rect` and `*` are different steps, but what if we had supported `div p`?), the order is fixed by the code, not the stylesheet order.

3.  **No Inheritance in Cascade:**
    *   Inheritance is handled *after* the cascade (style application) is complete for a node, during the state update/inheritance phase (`rsvg_state_inherit` etc.). This is generally correct but worth noting.
