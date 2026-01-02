# Developing librsvg

## Standard Build Commands

We use Meson for building librsvg.

### Debug Build

```bash
meson setup build
meson compile -C build
```

### Release Build

```bash
meson setup build-release --buildtype=release
meson compile -C build-release
```

### AddressSanitizer (ASan) Build

```bash
meson setup build-asan -Db_sanitize=address,undefined -Db_lundef=false
meson compile -C build-asan
```

## Running Static Analysis

See [STATIC_ANALYSIS.md](STATIC_ANALYSIS.md) for detailed instructions on `clang-tidy` and `scan-build`.

## Updating Generated Artifacts

### GObject Introspection

If you change the public API, you may need to update the GObject Introspection artifacts. This is done automatically by Meson if `-Dintrospection=enabled` is used.

### GObject Enum Types

If you add new enums to `rsvg.h`, `librsvg-enum-types.*` will be updated automatically during the build process.

## Reftests

If you change rendering logic, you must ensure that `rsvg-test` still passes. If the changes are intentional, follow the instructions in `tests/README.md` to regenerate reference images.

## Release Checklist

- [ ] Check for any ABI/API breaks. Follow SONAME rules.
- [ ] Update version macros in `src/librsvg-features.h.in`.
- [ ] Update `meson.build` version.
- [ ] Verify that the `.pc` file (pkg-config) is correctly generated.
- [ ] Verify that all public headers are correctly installed and can be compiled.
- [ ] Ensure all tests pass in both debug and release builds.
