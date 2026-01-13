# TODO.md

## C-forked librsvg 2.40 → GTK4-compatible symbolic CSS + dynamic stylesheet support

---

## 0. Project goal

Make a **C fork of librsvg 2.40.x** behave correctly when GTK4 injects CSS for symbolic icons and recoloring.

That means:

* CSS selectors must be re-applied **after load**
* Node identity (tag / id / class / style) must be preserved
* `currentColor` and dependent paints must update when CSS changes
* GTK4 must be able to call `rsvg_handle_set_stylesheet()` without breaking

This is effectively a **partial back-port of the ≥2.48 style cascade model** into the 2.40 C engine.

---

## 1. Data model upgrades

### 1.1 Extend `RsvgNode`

The current node only stores:

* `RsvgState *state`
* `parent`
* `children`
* `type`
* `name` (borrowed from libxml and becomes invalid)

You must convert it into a **style-addressable DOM node**.

**Add fields:**

* [ ] `char *name`
  owned copy of element tag

* [ ] `char *id`
  owned copy of `id` attribute

* [ ] `char *klass`
  owned raw class string (`"warning important foo"`)

* [ ] `char *style_attr`
  owned raw `style="..."`

* [ ] `RsvgState *base_state` (optional but strongly recommended)
  snapshot of state **after parse-time attributes & inline style**, before CSS injection

* [ ] `guint has_style_info : 1`

**Why this exists**

Without this, you cannot re-evaluate CSS selectors because the parse-time property bag is gone.

---

### 1.2 Ownership fixes

* [ ] Change `rsvg_standard_element_start()` to use `g_strdup(name)`
  do not store libxml’s transient string

* [ ] Update `_rsvg_node_finalize()` to free:

  * name
  * id
  * klass
  * style_attr
  * base_state (if added)

---

## 2. Capture attributes during parsing

### 2.1 Add centralized capture hook

In `rsvg_standard_element_start()`:

* [ ] Call a helper `rsvg_node_save_style_info(node, atts)` that extracts:

  * `id`
  * `class`
  * `style`

This avoids touching every `rsvg_node_*_set_atts()`.

---

### 2.2 Store inline style for re-application

Inline styles are parsed **after CSS** in `rsvg_parse_style_attrs()`.

If you don’t store `style_attr`, then when you re-apply CSS you will break precedence.

* [ ] Capture `style` attribute verbatim
* [ ] Re-apply it after CSS selectors during restyle

---

## 3. Add a real restyle engine

### 3.1 New API surface

Implement:

```
void rsvg_handle_set_stylesheet(RsvgHandle *handle,
                               const char *css,
                               gssize length,
                               GError **error)
```

Responsibilities:

* Parse CSS into `handle->priv->css_props`
* Trigger a full tree restyle
* Mark rendering caches dirty if needed

This matches ≥2.48 behavior conceptually.

---

### 3.2 Tree traversal

Implement:

```
void rsvg_apply_styles_recursive(RsvgHandle *ctx, RsvgNode *node)
```

Must:

1. Reset node state if base_state exists
2. Re-apply CSS selectors
3. Re-apply inline style
4. Recurse into children

---

### 3.3 Selector re-application logic

Re-implement the logic from `rsvg_parse_style_attrs()`:

Apply selectors in this exact order:

1. `*`
2. `tag`
3. `.class`
4. `tag.class`
5. `#id`
6. `tag#id`
7. inline style

You must split `class="a b c"` into tokens and apply each.

Use `rsvg_lookup_apply_css_style()` for every selector key.

---

### 3.4 State reset strategy

If you do not reset old CSS, styles will accumulate incorrectly.

Two options:

#### Minimal (symbolic-icon focused)

* Before re-applying CSS:

  * Overwrite `state->current_color`
  * Clear color-derived paints
  * Leave geometry alone

Works for GTK icons because only color changes.

#### Correct

* [ ] Snapshot `base_state` after parse
* [ ] On restyle:

  * `state = clone(base_state)`
  * apply CSS
  * apply inline style

This mirrors ≥2.48 semantics.

---

## 4. Rendering correctness

GTK symbolic icons depend on:

* `currentColor`
* `fill: currentColor`
* `stroke: currentColor`

Tasks:

* [ ] Verify that color propagation recomputes paints
* [ ] Ensure gradients and patterns referencing `currentColor` update
* [ ] Ensure cached cairo surfaces get invalidated

---

## 5. ABI and GTK4 compatibility

GTK4 expects:

* `rsvg_handle_set_stylesheet`
* `rsvg_handle_render_document` to reflect new styles

Tasks:

* [ ] Export the new symbol
* [ ] Provide pkg-config that advertises it
* [ ] Keep ABI compatible with librsvg-2.0

---

## 6. gdk-pixbuf loader integration

GTK4 still uses gdk-pixbuf for many SVG loads.

Tasks:

* [ ] Make the pixbuf loader call your fork
* [ ] Ensure stylesheet injection flows into your new API
* [ ] Ensure loader cache works or is bypassed

---

## 7. Test suite

### 7.1 Symbolic icon tests

Create SVGs that use:

```
<path class="warning" fill="currentColor"/>
```

Then inject CSS:

```
.warning { color: red }
```

Validate:

* Before CSS → default color
* After CSS → red

---

### 7.2 Regression tests

* Multiple classes
* id selectors
* tag + class
* inline style overrides CSS

Compare output vs ≥2.48 librsvg or browser.

---

## 8. Documentation

Write:

* How dynamic CSS works in your fork
* What subset of CSS is supported
* How it differs from stock 2.40
* How it differs from ≥2.48

This matters for packagers and GTK maintainers.

---

## 9. Performance and scaling

Restyling walks the entire tree.

Future work:

* Skip nodes that have no class/id
* Cache selector matches
* Track CSS dirty flags
