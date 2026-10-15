
from pathlib import Path

# Find the main project folder.
project = Path(__file__).resolve().parent.parent

# Locate the original PNG.
input_file = project / "assets" / "backgrounds" / "background1.png"

# Decide where the generated C++ header will go.
output_file = project / "include" / "background1.h"

# Read the entire PNG as binary data.
data = input_file.read_bytes()

# Convert every byte into C++ hexadecimal notation.
hex_bytes = [f"0x{byte:02X}" for byte in data]

# Put 12 bytes on each line to keep the file readable.
lines = [
    "    " + ", ".join(hex_bytes[i:i + 12])
    for i in range(0, len(hex_bytes), 12)
]

# Generate the C++ header.
header = (
    "#pragma once\n"
    "#include <stdint.h>\n"
    "#include <stddef.h>\n\n"
    "static const uint8_t background1_png[] = {\n"
    + ",\n".join(lines)
    + "\n};\n\n"
    "static const size_t background1_png_len = sizeof(background1_png);\n"
)

# Save the generated header.
output_file.write_text(header, encoding="utf-8")

print(f"Created: {output_file}")
print(f"Image size: {len(data)} bytes")
