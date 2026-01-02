# TODO

---

## Build and CI

### CI wiring (Meson-first, matches your `build-meson-asan/` workflow)

* [x] Add `.github/workflows/ci.yml` with jobs that run **from a clean build dir** (don’t reuse `build-meson-asan/`)

  * [x] `clang` job: `meson setup build-clang ... && meson compile -C build-clang && meson test -C build-clang --print-errorlogs`
  * [x] `gcc` job: same
  * [x] `asan/ubsan` job: `-Db_sanitize=address,undefined -Db_lundef=false` (if needed) and run full `meson test`
  * [x] `debugoptimized` job to catch “works in debug only” bugs
  * [x] `-Dintrospection=true/false` toggle jobs (you already generate `librsvg-enum-types.*` and GIR/typelib when enabled)
  * [x] `-Dpixbuf-loader=true/false` (if you add an option) to isolate loader failures
* [x] Add a CI step that prints dependency versions (glib, pango, cairo, libxml2, fontconfig, freetype) to help triage
* [x] Add a `ci` Meson option (or use `--werror`) so CI can be stricter than local dev

### Meson build ergonomics

* [ ] In root `meson.build`, add build targets/helpers:

  * [x] `ninja -C build test-asan` style alias via `meson.add_test_setup()` named setups: `asan`, `ubsan`, `asan_ubsan`
  * [x] `clang-tidy` convenience target that requires `compile_commands.json`
  * [x] `scan-build` convenience target (or documented script in `tests/`)
  
### Artifact sanity in CI

* [x] Add a minimal “installed headers compile test” job:

  * [x] `meson install` into DESTDIR
  * [x] compile a tiny program including `#include <librsvg/rsvg.h>` + linking `pkg-config --cflags --libs librsvg-2.0`

---

## Code health

### libxml2 SAX warning cleanup (in `src/rsvg-xml.c`, `src/rsvg-xml.h`)

* [x] Replace deprecated SAX usage that triggers warnings on new libxml2

  * [x] wrap libxml2 setup in a small internal module so only `rsvg-xml.c` touches libxml2 quirks
  * [x] fix `xmlStructuredErrorFunc` signature mismatch in `src/rsvg-css.c` for libxml2 >= 2.16
* [x] Make parse policy explicit and testable:

  * [x] disable external entity resolution and network access
  * [x] ensure no XInclude processing for untrusted inputs (you have `tests/fixtures/xinclude.svg` and `secret.txt` already)
* [x] Add “error path determinism”:

  * [x] consistent `GError` domain/message for parse failures so tests don’t flap

### GObject modernization (in `src/rsvg-gobject.c`, `src/rsvg-private.h`)

* [x] Convert `RsvgHandle` to `G_DEFINE_TYPE_WITH_PRIVATE` if not already
* [x] Use `g_autoptr`, `g_autofree`, `g_clear_object`, `g_clear_pointer` in new/refactored code paths
* [ ] Keep ABI padding in public structs intact (do not touch the `_abi_padding[15]`)
* [x] Reduce manual cleanup ladders in:

  * [x] `src/rsvg.c` (public API entrypoints)
  * [x] `src/rsvg-io.c` (GIO streams)

### Static analysis

* [x] Add documented entrypoints (no new dir needed):

  * [x] `doc/STATIC_ANALYSIS.md` with:

    * `meson setup build && meson compile -C build`
    * `ninja -C build clang-tidy` (or script)
    * `scan-build meson compile -C build`
* [x] Keep clang-tidy config in repo root: `.clang-tidy` with a minimal rule set
* [ ] Prefer fixing over suppressing; only suppress third-party/libcroco if needed

---

## Tests (fit your existing `tests/` layout)

You already have a strong base: `crash.c`, `render-crash.c`, `security-check.c`, `vulnerability_check.c`, `xss_checks.c`, `styles.c`, plus fixtures and reftests.

### Test execution hardening (in `tests/meson.build`)

* [x] Add Meson test setups:

  * [x] `asan_ubsan` with:

    * `ASAN_OPTIONS=halt_on_error=1:detect_leaks=1:strict_string_checks=1`
    * `UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`
  * [x] `glib_debug` with:

    * `G_SLICE=always-malloc`
    * `G_DEBUG=gc-friendly`
* [x] Ensure CI uses those setups: `meson test --setup=asan_ubsan --print-errorlogs`

### Expand existing suites without inventing a new framework

* [x] `tests/security-check.c`

  * [x] add explicit cases for:

    * recursion depth caps (`tests/fixtures/errors/308-*`)
    * gzip bomb-ish content (`tests/fixtures/errors/*svgz`)
    * xinclude denial (`tests/fixtures/xinclude.svg` + `tests/fixtures/secret.txt`)
* [x] `tests/xss_checks.c`

  * [x] add more `javascript:` variants and mixed-case, whitespace tricks
  * [x] add tests for event handler attributes across namespaces
* [x] `tests/vulnerability_check.c`

  * [x] add “budget” asserts:

    * too many elements
    * too complex paths
    * too large images (if supported)
  * [x] test both default limits and `RSVG_HANDLE_FLAG_UNLIMITED`

### Reftests / golden image stability (in `tests/fixtures/reftests`)

* [x] Create a simple rule for updating refs:

  * [x] add `tests/README.md` section: how to regenerate `*-ref.png`
* [x] Add a “pixel diff tolerance” policy:

  * [x] if you’re doing exact match today, keep exact
  * [x] if you see platform drift, add a tiny tolerance but keep it deterministic (document it)

### Add “API contract” tests (small but valuable)

* [x] Extend `tests/errors.c` and/or add a new `tests/api.c`:

  * [x] verify GError domain is `RSVG_ERROR`
  * [x] verify NULL/TRUE/FALSE conventions match doc for each public call
  * [x] verify incremental loading (`rsvg_handle_write` + `close`) errors are sticky

### Fuzz entrypoints

* [x] Add `tests/fuzz/` (new folder under `tests/`) with build-optional harnesses:

  * [x] fuzz: `rsvg_handle_new_from_data` + `render_cairo` to small surface
  * [x] fuzz: CSS input routed into libcroco adapter (see below)
  * [x] fuzz: path `d` parser (if you can isolate)
* [x] Gate behind Meson option `-Dfuzzing=true` so distros can ignore

---

## Modernizing libcroco (in `libcroco/`)

You already build it as an internal static (`build-meson-asan/libcroco/libcroco.a`). The goal is: **reduce warnings/UB, cap worst-case behavior, and isolate usage**, without changing public librsvg API.

### Step 1: Isolate all libcroco use behind one adapter (in `src/rsvg-css.c` / `src/rsvg-css.h`)

* [x] Make `src/rsvg-css.c` the only translation unit that includes `libcroco/src/libcroco.h` (or similar)
* [x] Define a small internal interface in `src/rsvg-css.h` that the rest of librsvg calls

  * parse stylesheet from bytes
  * apply to a node/style context
  * free stylesheet
* [x] Convert any direct libcroco type usage elsewhere into opaque pointers passed through the adapter

This makes future replacement feasible and makes fuzzing focused.

### Step 2: Bring libcroco compilation under stricter hygiene (in `libcroco/meson.build`)

* [x] Build libcroco with warnings that don’t drown you but catch real bugs:

  * [x] at least `-Wall -Wextra` equivalent via Meson warning_level
  * [x] add targeted flags for UB hotspots if manageable
* [x] Fix the highest-value warning classes first inside `libcroco/src/`:

  * [x] missing prototypes / implicit declarations
  * [x] signed/unsigned size comparisons
  * [x] truncation in token/length handling
  * [x] unchecked multiplication for allocations
* [x] Add small “safe size math” helpers in libcroco (internal) and use them for allocations

### Step 3: Add parsing caps to prevent CSS DoS (enforced in adapter, not sprinkled)

* [x] In `src/rsvg-css.c`, enforce:

  * [x] max CSS bytes
  * [x] max rules
  * [x] max selector length / complexity
  * [x] max declarations per rule
* [x] Add tests in `tests/styles.c` using existing fixtures or add new ones in:

  * `tests/fixtures/styles/`

    * [x] “too-many-rules.css-in-svg” style cases
    * [x] pathological selectors

### Step 4: Fuzz the adapter boundary

* [x] Add a fuzz harness (under `tests/fuzz/`) that feeds random CSS into the adapter
* [x] Run it at least in nightly CI or locally, not necessarily per-PR

### Step 5: incremental refactor inside libcroco

* [ ] Reduce global state and make error returns explicit where easy
* [ ] Make tokenizer/parser functions accept length-bounded spans, not rely on NUL-terminated strings
* [ ] Add overflow checks in:

  * `libcroco/src/cr-tknzr.c`
  * `libcroco/src/cr-parser.c`
  * `libcroco/src/cr-om-parser.c`

---

## gdk-pixbuf-loader (in `gdk-pixbuf-loader/`)

* [x] Add a dedicated loader test job:

  * [x] build loader
  * [x] run `gdk-pixbuf-loader/test.c` (you already have `test.c`)
  * [x] validate it can load:

    * `tests/fixtures/loading/gnome-cool.svg`
    * `tests/fixtures/loading/gnome-cool.svgz`
* [x] Add a security test ensuring loader respects the same limits as library defaults

---

## Docs and developer workflow (fit your existing docs)

You already have `doc/` plus `librsvg-api-abi.md`.

* [x] Expand `tests/README.md`:

  * [x] how to run tests + sanitizer setups
  * [x] how fixtures map to test binaries (crash, errors, dimensions, styles, xss, security-check)
  * [x] how to regenerate ref pngs in `tests/fixtures/reftests/**`
* [x] Add `doc/DEVELOPING.md` or extend existing doc:

  * [x] standard build commands (debug/release/asan)
  * [x] how to run clang-tidy/scan-build
  * [x] how to update generated headers and introspection artifacts
* [x] Add a short “release checklist” that mentions:

  * [x] SONAME rules
  * [x] updating `src/librsvg-features.h.in` version macros
  * [x] verifying `.pc` and installed headers
