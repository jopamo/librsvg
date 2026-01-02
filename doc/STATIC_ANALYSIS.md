# Static Analysis

To run static analysis on librsvg, you can use the following tools:

## clang-tidy

First, ensure you have a build directory with a compilation database:

```bash
meson setup build
meson compile -C build
```

Then run the `clang-tidy` target:

```bash
ninja -C build clang-tidy
```

The configuration is maintained in the `.clang-tidy` file in the root directory.

## scan-build

Meson provides a built-in `scan-build` target:

```bash
ninja -C build scan-build
```

Alternatively, you can run `scan-build` manually:

```bash
scan-build meson compile -C build
```

Or use our convenience target:

```bash
ninja -C build scan-build-analysis
```
