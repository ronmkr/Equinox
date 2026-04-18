# Equinox

**Equinox** is a system-wide, low-latency audio equalization and processing application for macOS designed for audiophiles.

## Core Features
- **31-band Graphic and Parametric EQ**: High-precision filters using JUCE `dsp` module.
- **AutoEQ/Squiglink curve import**: Direct parsing of standard AutoEQ strings.
- **VST3/AU Plugin Hosting**: Integrated `AudioProcessorGraph` for serial plugin chains.
- **Safety Limiter**: Real-time `dsp::Limiter` at the end of the signal path to prevent output clipping.
- **Profile Management**: Fast profile switching via SQLite with gapless A/B comparison.
- **Multi-Tab Interface**: Dedicated views for EQ, Plugins, and Device Settings.

## Architecture
Equinox uses a modular graph-based architecture:
1.  **Audio Input**: macOS CoreAudio loopback capture.
2.  **EQ Engine**: 31-band biquad cascade (`FilterProcessor`).
3.  **Plugin Chain**: Serially hosted external VST3/AU plugins.
4.  **Limiter**: Final stage protection (`LimiterProcessor`).
5.  **Audio Output**: Physical hardware output.

## Tech Stack
- **Framework:** JUCE 8 (C++20)
- **Engine:** `juce::AudioProcessorGraph`, JUCE `dsp` module.
- **UI:** Custom JUCE Components with modern RAII layouts.
- **Database:** SQLite for profile persistence.
- **Build System:** CMake.

## Getting Started
### Prerequisites
- macOS 12.0 or later.
- CMake 3.22+.

### Build Instructions
```bash
mkdir build && cd build
cmake ..
cmake --build .
```
The application will be generated in the `build/` directory.

## Contributing
Please refer to `GEMINI.md` for real-time safety mandates and coding standards before submitting PRs.
