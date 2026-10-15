# Convert original PNGs into reproducible RGB565 headers under matching include folders.
from pathlib import Path
import re
from PIL import Image

PROJECT = Path(__file__).resolve().parent.parent
TRANSPARENT = 0xF81F


def rgb565(red, green, blue):
    """Quantise an RGB colour into the display's 16-bit format."""
    return ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3)


def convert(source):
    """Convert one PNG; animal canvases use binary alpha and a feet anchor."""
    relative = source.relative_to(PROJECT / "assets")
    animal = relative.parts[0] == "animals"
    with Image.open(source) as original:
        image = original.convert("RGBA")
    if animal:
        # Standardise the supplied 48x48 Dodo to the shared 24x24 game canvas.
        if image.size not in ((24, 24), (48, 48)):
            raise ValueError(f"{relative}: animal canvas must be 24x24 or 48x48")
        image = image.resize((24, 24), Image.Resampling.NEAREST)
    elif image.size != (320, 240):
        raise ValueError(f"{relative}: background must be 320x240")

    pixels = []
    opaque_rows = []
    for index, (red, green, blue, alpha) in enumerate(image.getdata()):
        if not animal and alpha != 255:
            raise ValueError(f"{relative}: backgrounds must be fully opaque")
        # RGB565 colour-key sprites have binary transparency, not partial alpha.
        if animal and alpha < 128:
            pixels.append(TRANSPARENT)
        else:
            colour = rgb565(red, green, blue)
            # Reserve the key: opaque magenta remains visible as its nearest
            # RGB565 neighbour, differing by one blue level, never a hole.
            if animal and colour == TRANSPARENT:
                colour = 0xF81E
            pixels.append(colour)
            opaque_rows.append(index // image.width)

    symbol = re.sub(r"[^a-zA-Z0-9_]", "_", relative.with_suffix("").as_posix())
    output = PROJECT / "include" / relative.with_suffix(".h")
    output.parent.mkdir(parents=True, exist_ok=True)
    lines = [
        f"// Generated from assets/{relative.as_posix()} by tools/convert_image.py; do not edit.",
        "#pragma once",
        "#include <stdint.h>",
        "",
        f"static constexpr int {symbol}_width = {image.width};",
        f"static constexpr int {symbol}_height = {image.height};",
    ]
    if animal:
        if not opaque_rows:
            raise ValueError(f"{relative}: animal must have visible pixels")
        lines += [
            f"static constexpr int {symbol}_anchor_x = {image.width // 2};",
            f"static constexpr int {symbol}_anchor_y = {max(opaque_rows) + 1};",
            f"static constexpr uint16_t {symbol}_transparent = 0xF81F;",
        ]
    lines += [f"static const uint16_t {symbol}_pixels[] = {{"]
    for start in range(0, len(pixels), 12):
        lines.append("    " + ", ".join(f"0x{pixel:04X}" for pixel in pixels[start:start + 12]) + ",")
    lines += ["};", ""]
    output.write_text("\n".join(lines), encoding="utf-8", newline="\n")
    print(f"Created {output.relative_to(PROJECT)} ({image.width}x{image.height})")


def main():
    """Convert in stable path order; original PNG files are never modified."""
    for source in sorted((PROJECT / "assets").rglob("*.png")):
        convert(source)


if __name__ == "__main__":
    main()
