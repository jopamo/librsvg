#!/usr/bin/env python3
import json
import os
import sys


def fail(message):
    sys.stderr.write(message + "\n")
    raise SystemExit(1)


def load_targets(build_root):
    intro_path = os.path.join(build_root, "meson-info", "intro-targets.json")
    if not os.path.exists(intro_path):
        fail("Missing meson introspection data at: " + intro_path)
    with open(intro_path, "r", encoding="utf-8") as handle:
        return json.load(handle)


def has_target(targets, name):
    return any(target.get("name") == name for target in targets)


def require_compile_commands(build_root):
    compile_commands = os.path.join(build_root, "compile_commands.json")
    if not os.path.exists(compile_commands):
        fail("Missing compile_commands.json at: " + compile_commands)


def main():
    build_dir = os.environ.get("G_TEST_BUILDDIR")
    if not build_dir:
        fail("G_TEST_BUILDDIR is not set")
    build_root = os.path.dirname(build_dir)
    targets = load_targets(build_root)

    expect_clang_tidy = os.environ.get("RSVG_EXPECT_CLANG_TIDY") == "1"
    expect_scan_build = os.environ.get("RSVG_EXPECT_SCAN_BUILD") == "1"

    if expect_clang_tidy and not has_target(targets, "clang-tidy"):
        fail("Expected clang-tidy target to be present")
    if not expect_clang_tidy and has_target(targets, "clang-tidy"):
        fail("clang-tidy target present without expected tool")
    if expect_clang_tidy:
        require_compile_commands(build_root)

    if expect_scan_build and not has_target(targets, "scan-build-analysis"):
        fail("Expected scan-build-analysis target to be present")
    if not expect_scan_build and has_target(targets, "scan-build-analysis"):
        fail("scan-build-analysis target present without expected tool")


if __name__ == "__main__":
    main()
