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
* [ ] Add a `ci` Meson option (or use `--werror`) so CI can be stricter than local dev

### Meson build ergonomics

* [ ] In root `meson.build`, add build targets/helpers:

  * [ ] `ninja -C build test-asan` style alias via `meson.add_test_setup()` named setups: `asan`, `ubsan`, `asan-ubsan`
  * [ ] `clang-tidy` convenience target that requires `compile_commands.json`
  * [ ] `scan-build` convenience target (or documented script in `tests/`)
* [ ] Document “known good” toolchains in `doc/` or `TODO.md`:

  * [ ] minimum Meson/Ninja/clang/gcc versions you expect

### Artifact sanity in CI

* [ ] Add a minimal “installed headers compile test” job:

  * [ ] `meson install` into DESTDIR
  * [ ] compile a tiny program including `#include <librsvg/rsvg.h>` + linking `pkg-config --cflags --libs librsvg-2.0`

---

## Code health

### libxml2 SAX warning cleanup (in `src/rsvg-xml.c`, `src/rsvg-xml.h`)

* [ ] Replace deprecated SAX usage that triggers warnings on new libxml2

  * [ ] wrap libxml2 setup in a small internal module so only `rsvg-xml.c` touches libxml2 quirks
* [ ] Make parse policy explicit and testable:

  * [ ] disable external entity resolution and network access
  * [ ] ensure no XInclude processing for untrusted inputs (you have `tests/fixtures/xinclude.svg` and `secret.txt` already)
* [ ] Add “error path determinism”:

  * [ ] consistent `GError` domain/message for parse failures so tests don’t flap

### GObject modernization (in `src/rsvg-gobject.c`, `src/rsvg-private.h`)

* [ ] Convert `RsvgHandle` to `G_DEFINE_TYPE_WITH_PRIVATE` if not already
* [ ] Use `g_autoptr`, `g_autofree`, `g_clear_object`, `g_clear_pointer` in new/refactored code paths
* [ ] Keep ABI padding in public structs intact (do not touch the `_abi_padding[15]`)
* [ ] Reduce manual cleanup ladders in:

  * [ ] `src/rsvg.c` (public API entrypoints)
  * [ ] `src/rsvg-io.c` (GIO streams)

### Static analysis

* [ ] Add documented entrypoints (no new dir needed):

  * [ ] `doc/STATIC_ANALYSIS.md` with:

    * `meson setup build && meson compile -C build`
    * `ninja -C build clang-tidy` (or script)
    * `scan-build meson compile -C build`
* [ ] Keep clang-tidy config in repo root: `.clang-tidy` with a minimal rule set
* [ ] Prefer fixing over suppressing; only suppress third-party/libcroco if needed

---

## Tests (fit your existing `tests/` layout)

You already have a strong base: `crash.c`, `render-crash.c`, `security-check.c`, `vulnerability_check.c`, `xss_checks.c`, `styles.c`, plus fixtures and reftests.

### Test execution hardening (in `tests/meson.build`)

* [ ] Add Meson test setups:

  * [ ] `asan-ubsan` with:

    * `ASAN_OPTIONS=halt_on_error=1:detect_leaks=1:strict_string_checks=1`
    * `UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`
  * [ ] `glib-debug` with:

    * `G_SLICE=always-malloc`
    * `G_DEBUG=gc-friendly`
* [ ] Ensure CI uses those setups: `meson test --setup=asan-ubsan --print-errorlogs`

### Expand existing suites without inventing a new framework

* [ ] `tests/security-check.c`

  * [ ] add explicit cases for:

    * recursion depth caps (`tests/fixtures/errors/308-*`)
    * gzip bomb-ish content (`tests/fixtures/errors/*svgz`)
    * xinclude denial (`tests/fixtures/xinclude.svg` + `tests/fixtures/secret.txt`)
* [ ] `tests/xss_checks.c`

  * [ ] add more `javascript:` variants and mixed-case, whitespace tricks
  * [ ] add tests for event handler attributes across namespaces
* [ ] `tests/vulnerability_check.c`

  * [ ] add “budget” asserts:

    * too many elements
    * too complex paths
    * too large images (if supported)
  * [ ] test both default limits and `RSVG_HANDLE_FLAG_UNLIMITED`

### Reftests / golden image stability (in `tests/fixtures/reftests`)

* [ ] Create a simple rule for updating refs:

  * [ ] add `tests/README.md` section: how to regenerate `*-ref.png`
* [ ] Add a “pixel diff tolerance” policy:

  * [ ] if you’re doing exact match today, keep exact
  * [ ] if you see platform drift, add a tiny tolerance but keep it deterministic (document it)

### Add “API contract” tests (small but valuable)

* [ ] Extend `tests/errors.c` and/or add a new `tests/api.c`:

  * [ ] verify GError domain is `RSVG_ERROR`
  * [ ] verify NULL/TRUE/FALSE conventions match doc for each public call
  * [ ] verify incremental loading (`rsvg_handle_write` + `close`) errors are sticky

### Fuzz entrypoints (optional but prepare hooks)

* [ ] Add `tests/fuzz/` (new folder under `tests/`) with build-optional harnesses:

  * [ ] fuzz: `rsvg_handle_new_from_data` + `render_cairo` to small surface
  * [ ] fuzz: CSS input routed into libcroco adapter (see below)
  * [ ] fuzz: path `d` parser (if you can isolate)
* [ ] Gate behind Meson option `-Dfuzzing=true` so distros can ignore

---

## Modernizing libcroco (in `libcroco/`)

You already build it as an internal static (`build-meson-asan/libcroco/libcroco.a`). The goal is: **reduce warnings/UB, cap worst-case behavior, and isolate usage**, without changing public librsvg API.

### Step 1: Isolate all libcroco use behind one adapter (in `src/rsvg-css.c` / `src/rsvg-css.h`)

* [ ] Make `src/rsvg-css.c` the only translation unit that includes `libcroco/src/libcroco.h` (or similar)
* [ ] Define a small internal interface in `src/rsvg-css.h` that the rest of librsvg calls

  * parse stylesheet from bytes
  * apply to a node/style context
  * free stylesheet
* [ ] Convert any direct libcroco type usage elsewhere into opaque pointers passed through the adapter

This makes future replacement feasible and makes fuzzing focused.

### Step 2: Bring libcroco compilation under stricter hygiene (in `libcroco/meson.build`)

* [ ] Build libcroco with warnings that don’t drown you but catch real bugs:

  * [ ] at least `-Wall -Wextra` equivalent via Meson warning_level
  * [ ] add targeted flags for UB hotspots if manageable
* [ ] Fix the highest-value warning classes first inside `libcroco/src/`:

  * [ ] missing prototypes / implicit declarations
  * [ ] signed/unsigned size comparisons
  * [ ] truncation in token/length handling
  * [ ] unchecked multiplication for allocations
* [ ] Add small “safe size math” helpers in libcroco (internal) and use them for allocations

### Step 3: Add parsing caps to prevent CSS DoS (enforced in adapter, not sprinkled)

* [ ] In `src/rsvg-css.c`, enforce:

  * [ ] max CSS bytes
  * [ ] max rules
  * [ ] max selector length / complexity
  * [ ] max declarations per rule
* [ ] Add tests in `tests/styles.c` using existing fixtures or add new ones in:

  * `tests/fixtures/styles/`

    * [ ] “too-many-rules.css-in-svg” style cases
    * [ ] pathological selectors

### Step 4: Fuzz the adapter boundary

* [ ] Add a fuzz harness (under `tests/fuzz/`) that feeds random CSS into the adapter
* [ ] Run it at least in nightly CI or locally, not necessarily per-PR

### Step 5: Optional incremental refactor inside libcroco

* [ ] Reduce global state and make error returns explicit where easy
* [ ] Make tokenizer/parser functions accept length-bounded spans, not rely on NUL-terminated strings
* [ ] Add overflow checks in:

  * `libcroco/src/cr-tknzr.c`
  * `libcroco/src/cr-parser.c`
  * `libcroco/src/cr-om-parser.c`

---

## gdk-pixbuf-loader (in `gdk-pixbuf-loader/`)

* [ ] Add a dedicated loader test job:

  * [ ] build loader
  * [ ] run `gdk-pixbuf-loader/test.c` (you already have `test.c`)
  * [ ] validate it can load:

    * `tests/fixtures/loading/gnome-cool.svg`
    * `tests/fixtures/loading/gnome-cool.svgz`
* [ ] Add a security test ensuring loader respects the same limits as library defaults

---

## Docs and developer workflow (fit your existing docs)

You already have `doc/` plus `librsvg-api-abi.md`.

* [ ] Expand `tests/README.md`:

  * [ ] how to run tests + sanitizer setups
  * [ ] how fixtures map to test binaries (crash, errors, dimensions, styles, xss, security-check)
  * [ ] how to regenerate ref pngs in `tests/fixtures/reftests/**`
* [ ] Add `doc/DEVELOPING.md` or extend existing doc:

  * [ ] standard build commands (debug/release/asan)
  * [ ] how to run clang-tidy/scan-build
  * [ ] how to update generated headers and introspection artifacts
* [ ] Add a short “release checklist” that mentions:

  * [ ] SONAME rules
  * [ ] updating `src/librsvg-features.h.in` version macros
  * [ ] verifying `.pc` and installed headers
