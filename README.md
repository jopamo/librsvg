# librsvg (C-only continuation)

This repository is a continuation of the librsvg 2.40 C codebase with a focus on
keeping GUI environments working without a hard Rust dependency. It preserves
the public API/ABI while modernizing the C implementation, tests, and tooling.

## Goals

- Keep the 2.40-era C implementation viable for C/GLib-based stacks.
- Maintain ABI stability for the `librsvg-2.0` API surface.
- Improve safety, limits, and test coverage without changing public behavior.

## Why C-only

This fork exists to keep librsvg usable in GUI environments that cannot or do
not want to take a hard Rust dependency. Scope is limited to the 2.40 C codebase
and its public API/ABI; the goal is compatibility and maintenance, not a full
re-architecture or feature divergence.

## Build (Meson)

Debug build:

```bash
meson setup build
meson compile -C build
```

Release build:

```bash
meson setup build-release --buildtype=release
meson compile -C build-release
```

ASan/UBSan build:

```bash
meson setup build-asan -Db_sanitize=address,undefined -Db_lundef=false
meson compile -C build-asan
```

## Tests

```bash
meson test -C build --print-errorlogs
```

See `tests/README.md` for reftests and fixture details.

## Documentation

- Developer workflow: `doc/DEVELOPING.md`
- Static analysis: `doc/STATIC_ANALYSIS.md`
- API/ABI reference: `librsvg-api-abi.md`

## Project layout (high level)

- `src/`: core library implementation
- `libcroco/`: bundled CSS parser used by the renderer
- `gdk-pixbuf-loader/`: optional gdk-pixbuf loader
- `tests/`: unit tests, security tests, and fixtures

## License

See `COPYING`.
