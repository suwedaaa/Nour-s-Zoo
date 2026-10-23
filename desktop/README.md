# Nour's Zoo desktop simulator

This Windows simulator compiles the actual game, rendering, input and sound C++ sources. A small adapter implements the subset of M5Unified used by the game. It is not an ESP32 CPU emulator.

## Run

From the project folder in PowerShell:

```powershell
./tools/run-desktop.ps1
```

Requires CMake and Visual Studio C++ Build Tools with the Windows SDK (both were present on the development machine). Alternatively, open `build/desktop/Release/NoursZoo.exe` after building. Re-run the script after changing game code.

## Controls and signals

- A / S / D correspond to the Fire's A / B / C buttons. Mouse buttons can be held too. Losing window focus releases inputs.
- Short S interacts; hold S for at least 700 ms on the zoo screen to open sound settings.
- M mutes PC playback independently of the game's volume. Output commands remain visible.
- T sends a labelled test tone; L toggles a labelled ten-LED strip test; V sends a labelled vibration signal test.
- The event panel keeps the newest events visible, including brief sound pulses. The console retains a longer transcript.

Speaker channels use asynchronous Windows PCM playback. Frequency and duration follow game commands, with a sine-wave approximation and a short fade to reduce clicks. PC audio does not reproduce the physical speaker's timbre or loudness. Volume changes affect subsequently started tones.

There is currently no LED or vibration gameplay in the project. These indicators start idle and diagnostic events are labelled TEST. `desktop/Signals.h` exposes the desktop signal hooks for future output adapters; adding real hardware effects also requires a corresponding hardware implementation. Vibration is a logical output visualization, not a claim that the stock Fire has a vibration motor.

The renderer uses RGB565 artwork and the bundled, BSD-licensed 5x7 font from M5GFX, with its license retained in `font5x7.h`. The compatibility layer supports only the APIs used here; new hardware/library features may require extending it. ESP32 memory, peripheral timing, power and sensors are not emulated. Game state remains in memory, as in the current firmware.

## Verify

```powershell
cmake --build build/desktop --config Release
ctest --test-dir build/desktop -C Release --output-on-failure
```

Tests cover the desktop framebuffer/input/output smoke path plus existing button timing, dialogue, timer and sound tests. The smoke test generates `build/desktop/desktop-preview.bmp` using the same paint function as the window. It does not verify audible output on a physical PC speaker.

The original firmware remains buildable with `pio run -e m5stack-fire`.
