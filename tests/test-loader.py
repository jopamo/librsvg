#!/usr/bin/env python3
import os
import subprocess
import sys
import shutil

def main():
    if len(sys.argv) < 4:
        print("Usage: test-loader.py <rsvg-loader-exe> <loader-so> <svg-file> <output-png>")
        sys.exit(1)

    loader_exe = sys.argv[1]
    loader_so = sys.argv[2]
    svg_file = sys.argv[3]
    output_png = sys.argv[4]

    # Create a temporary loaders file
    loaders_file = "gdk-pixbuf.loaders"
    with open(loaders_file, "w") as f:
        f.write(f'"{loader_so}"\n')
        f.write('"svg" 2 "librsvg" "Scalable Vector Graphics" "LGPL"\n')
        f.write('"image/svg+xml" "image/svg" "image/svg-xml" "image/vnd.adobe.svg+xml" "text/xml-svg" "image/svg+xml-compressed" ""\n')
        f.write('"svg" "svgz" "svg.gz" ""\n')
        f.write('" <svg" "*    " 100\n')
        f.write('" <!DOCTYPE svg" "*             " 100\n')
        f.write('"\\x1f\\x8b\\x08" "   " 10\n')
        f.write('\n')

    env = os.environ.copy()
    env["GDK_PIXBUF_MODULE_FILE"] = loaders_file

    try:
        subprocess.check_call([loader_exe, svg_file, output_png], env=env)
        print(f"Successfully loaded {svg_file} and saved to {output_png}")
    except subprocess.CalledProcessError as e:
        print(f"Failed to load {svg_file}: {e}")
        sys.exit(1)
    finally:
        if os.path.exists(loaders_file):
            os.remove(loaders_file)

if __name__ == "__main__":
    main()
