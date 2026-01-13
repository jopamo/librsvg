# librsvg (C-only continuation)

This repository is a modernized continuation of the librsvg 2.40 C codebase. It is designed to provide a robust, secure, and API-compatible SVG rendering library for environments where a Rust toolchain is not available or desired.

It focuses on:
*   **GTK4 Compatibility:** Native support for symbolic icon recoloring and dynamic stylesheets.
*   **Modern Tooling:** A clean Meson build system, extensive CI integration, and sanitizer support.
*   **ABI Stability:** Drop-in replacement for `librsvg-2.0` (version 2.52.0 compatible).
*   **Safety:** Hardened XML parsing limits and extensive fuzzing/regression testing.

## Build

This project uses the [Meson](https://mesonbuild.com/) build system.

### Dependencies

*   glib-2.0 >= 2.12.0
*   gdk-pixbuf-2.0 >= 2.20
*   cairo >= 1.2.0
*   libxml-2.0 >= 2.9.0
*   pango >= 1.38.0
*   libcroco (bundled/integrated)

### Compilation

```bash
# fast debug build
meson setup build
meson compile -C build

# release build
meson setup build-release --buildtype=release
meson compile -C build-release
```

### Options

*   `-Dgdk_pixbuf_loader=true`: Build the gdk-pixbuf loader (default: true).
*   `-Dintrospection=enabled`: Build GObject Introspection data.
*   `-Dgtk_demo=enabled`: Build GTK3 demo tools.

## Testing

The test suite covers API, rendering correctness, security limits, and regressions.

```bash
meson test -C build --print-errorlogs
```

For advanced testing (ASan/UBSan), use the predefined setups:

```bash
meson setup build-san --setup=asan_ubsan
meson test -C build-san
```

See `tests/TESTING.md` for detailed information on the test suite structure and adding new regression tests.

## GTK4 Compatibility and Dynamic CSS

This fork includes a back-ported implementation of `rsvg_handle_set_stylesheet` (introduced in librsvg 2.48) to support GTK4 symbolic icon recoloring.

Unlike the original 2.40.x codebase which resolved styles only during parsing, this version captures node metadata (tag names, classes, IDs) and supports a full tree restyle when a new stylesheet is injected. This ensures that `currentColor` and class-based symbolic styling work as expected in modern GTK4 environments.

### Supported CSS

The CSS support is powered by `libcroco` (as in stock 2.40) and covers standard SVG 1.1 styling attributes and CSS2 selectors.
*   Selectors: `*`, `tag`, `.class`, `#id`, and combinations.
*   Properties: Standard SVG presentation attributes (fill, stroke, etc.).
*   **Differences from 2.48+**: This C implementation does not support the full CSS3/CSS4 feature set found in the Rust version (e.g., complex pseudo-classes, variables other than `currentColor`).

## Project Layout

- `src/`: Core library implementation (GObject-based).
- `libcroco/`: Integrated CSS parser.
- `gdk-pixbuf-loader/`: GdkPixbuf module for SVG support.
- `tests/`: Unit tests, rendering tests (reftests), and fuzzing corpus.

## License

See `COPYING`.
