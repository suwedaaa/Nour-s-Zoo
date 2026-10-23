# Nour's Zoo

M5Stack Fire game with a Windows desktop simulator.

## Run on Windows

Install CMake and Visual Studio C++ Build Tools with the Windows SDK. Run these commands from the project folder in PowerShell:

```powershell
./tools/run-desktop.ps1
```

The script builds and launches the simulator. After building, you can also open `build/desktop/Release/NoursZoo.exe` directly. Close the simulator before rebuilding, then re-run the script after code or generated artwork changes. Opening an old executable does not rebuild it.

If PowerShell blocks the script because of its execution policy, run this trusted project script for that process only:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ./tools/run-desktop.ps1
```

## Controls

- Desktop **A / S / D** map to the Fire's **A / B / C** buttons. You can also click and hold the on-screen buttons.
- Short **B** (desktop **S**) interacts or confirms. In menus, **A / C** move the selection. During dialogue, any short button press reveals or advances the text.
- Hold **B** for at least **700 ms** on the zoo screen to open sound settings.
- Choose **Study** from the action menu to start a **25-minute** timer. Press **B** to stop it and return to the menu.
- Desktop **M** toggles PC audio independently of the game's volume setting.
- Desktop **T / L / V** test a tone, the LED strip, and a vibration signal respectively. These are labelled simulator diagnostics.

The output panel records game tone frequency, duration and volume, so very brief sounds remain visible in the log. LED and vibration indicators are available for testing, but the game does not currently trigger those effects. See [desktop details](desktop/README.md) for output hooks and simulator limitations.

## Convert artwork

Edit the original PNG files under `assets/`, then use **`tools/convert_image.py`** to generate the C++ RGB565 image headers. Do not hand-edit the generated headers.

Python with Pillow is required. Install Pillow once into the Python environment you use for conversion:

```powershell
python -m pip install Pillow
```

After adding or changing artwork:

```powershell
python tools/convert_image.py
./tools/run-desktop.ps1
```

The converter takes **no command-line arguments**. It scans all `*.png` files recursively under `assets/` and overwrites the matching `.h` files under `include/`. It does not modify the original PNGs. For example:

- `assets/backgrounds/background1.png` generates `include/backgrounds/background1.h`.
- `assets/animals/dodo/dodo1.png` generates `include/animals/dodo/dodo1.h`.

Artwork requirements:

- **Backgrounds:** exactly **320 × 240**, fully opaque. Any PNG outside `assets/animals/` is currently treated as a background.
- **Animals:** **24 × 24** or **48 × 48**. Both become a **24 × 24** canvas; 48 × 48 images are downscaled with nearest-neighbour sampling.
- **Animal transparency:** alpha below 128 becomes transparent; alpha 128 or above becomes fully opaque. Partial transparency is not retained.
- **Colour key:** `0xF81F` is reserved for transparent animal pixels. Opaque colours that quantise to that value are adjusted automatically so they remain visible.
- **Positioning:** the converter generates a horizontal centre anchor and a feet anchor just below the lowest visible row. Animals must contain visible pixels.

Conversion is a separate manual step; neither the desktop nor firmware build runs it automatically. If conversion reports an invalid image, fix it and rerun before building: earlier files may already have been regenerated.

Adding a PNG generates a header but does **not** automatically add a new animal or scene to the game. Wire the new asset into `src/Renderer.cpp` and the relevant gameplay code. Renaming or deleting a PNG also does not remove its old generated header. Commit both the source PNG and its generated header when changing artwork.

## Build and upload to M5Stack Fire

Install PlatformIO, either through its VS Code extension or its CLI. The project uses the `m5stack-fire` environment, Arduino, M5Unified, a 16 MB flash partition layout and PSRAM support. PlatformIO resolves the configured firmware dependencies on build; the first build may need network access.

```powershell
# Build without connecting a device
pio run -e m5stack-fire

# Upload when the physical Fire is connected over USB
pio run -e m5stack-fire -t upload

# View serial diagnostics
pio device monitor -b 115200
```

If `pio` is not on your PATH, use the PlatformIO terminal in VS Code. If multiple serial devices are attached, specify the correct port with `--upload-port COMx` for upload or `--port COMx` for the monitor. Close the serial monitor if it prevents uploading.

## Tests

After configuring the desktop build with the run script:

```powershell
cmake --build build/desktop --config Release
ctest --test-dir build/desktop -C Release --output-on-failure
```

The five tests cover desktop rendering/input/output smoke checks, button timing, dialogue, study timers and sound commands. The smoke test creates `build/desktop/desktop-preview.bmp` using the window's paint function. Audible playback and physical hardware behaviour still need manual checks.

## Where to make changes

- `src/Game.cpp`: screen transitions and gameplay actions.
- `src/DialogueData.cpp`: dialogue scripts and choices.
- `src/Renderer.cpp`: screen layouts and artwork selection.
- `src/Input.cpp` and `src/ButtonState.cpp`: button sampling, debounce and short/long presses.
- `src/Sound.cpp`: sound effects and volume levels.
- `include/StudyTimer.h` and `src/StudyTimer.cpp`: study duration and countdown logic.
- `assets/`: source PNGs; matching folders under `include/` contain generated artwork.
- `desktop/`: Windows adapters, output signals and bundled bitmap font.
- `platformio.ini`: Fire firmware configuration; `CMakeLists.txt`: desktop build and tests.

## Current limitations

The desktop simulator runs the same C++ gameplay and rendering code through a small hardware compatibility layer. It does not emulate the ESP32 CPU, PSRAM, sensors, power usage or peripheral timing. New M5Unified calls may require extending the desktop adapter.

Game history, sound settings and progress are currently stored only in memory and reset when the application or device restarts. Bond progression and persistent saves are not implemented yet. Speaker playback on a PC approximates tones; it does not reproduce the physical Fire's loudness or timbre. Vibration is a logical signal display, not an indication that the stock Fire contains a vibration motor.

Build output lives in `build/` and `.pio/`; both are ignored by Git. Keep source changes and generated image headers in version control so builds can be recreated.
