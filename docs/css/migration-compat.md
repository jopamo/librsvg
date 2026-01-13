# CSS Migration Compatibility & Quirks Policy

## Overview

Replacing `libcroco` with `libcss` will transition `librsvg` from an ad-hoc, semi-compliant CSS engine to a standards-compliant one. This transition will inevitably introduce behavioral changes. This document defines the policy for handling these changes.

## Compatibility Bounds

### 1. Selectors
*   **Goal:** Full CSS Level 2 (and partial Level 3) selector support.
*   **Behavior Change:** **ACCEPTED**.
    *   `librsvg` currently ignores combinators (e.g., `div p`, `div > p`). Enabling them will change the rendering of SVGs that contain such selectors (previously ignored, now active). This is considered a **bug fix**.
    *   Specificity calculation will move from an ad-hoc "application order" (Universal -> Tag -> Class -> ID) to standard CSS specificity scoring.
    *   **Breakage:** Rare cases where the ad-hoc precedence differed from standard specificity (e.g., `tag#id` vs `#id` vs `tag.class` chains) will resolve to the standard behavior. This is **ACCEPTED**.

### 2. Properties & Values
*   **Goal:** Strict adherence to CSS syntax.
*   **Quirks to Drop:**
    *   **Simplistic Quoting:** `librsvg` currently strips all single quotes indiscriminately. `libcss` handles quotes correctly. Files relying on broken quoting (e.g., `font-family: 'Times New Roman` without closing quote) may fail to parse. This is **ACCEPTED**.
    *   **`!important` Parsing:** The current split-on-`!` logic will be replaced by standard parsing.
*   **Quirks to Preserve (Best Effort):**
    *   **Unitless Lengths:** SVG allows unitless lengths (interpreted as user units). `libcss` might expect units in some contexts. We must ensure unitless numbers are handled as SVG user units.
    *   **Presentation Attribute Precedence:** SVG-specific precedence (Presentation Attributes < Author Style) must be maintained.

## Testing Thresholds

### 1. Visual Regressions (`rsvg-test`)
*   **Target:** Pixel-exact matches for features that are fully supported and standard.
*   **Tolerance:**
    *   **Antialiasing:** Minor pixel differences (< 1%) due to precision changes (double vs fixed-point) or parsing differences (e.g. `10.5` vs `10.50`) are **PERMITTED** but must be manually verified and re-baselined.
    *   **Layout:** No layout shifts are permitted for supported features.

### 2. Style Dumps
*   **Target:** Deterministic output.
*   **Migration:**
    *   During the transition, `libcss` backend results will differ from `libcroco` baseline in terms of specificity and supported selectors.
    *   **Strategy:** We will maintain the "Baseline" (current behavior) as the reference until Phase 8.
    *   When `libcss` is enabled, we expect the style dump to show *more* matched properties (due to combinators) and potentially different cascade resolutions.

## Known Risks

1.  **Combinator Activation:** SVGs that include "web CSS" (e.g. Bootstrap) might suddenly have selectors matching that didn't match before, potentially breaking the visual appearance if the SVG wasn't designed for it.
2.  **Error Handling:** `libcroco` is lenient. `libcss` might be stricter. We should ensure the parser is configured to be recovering/lenient where possible.
