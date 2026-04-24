# Equinox

Equinox is a system-wide, low-latency audio equalization and processing application for macOS designed for audiophiles. It provides a modular environment for high-precision filtering, plugin hosting, and real-time audio analysis.

## Core Features

- **High-Precision EQ**: 31-band Graphic and Parametric EQ using high-fidelity biquad filters.
- **AutoEQ Integration**: Direct import of standard AutoEQ and Squiglink curves.
- **Plugin Hosting**: Support for VST3 and AU plugins via an integrated processing graph.
- **Real-time Analysis**: Integrated oscilloscope and parametric curve visualizer.
- **Safety First**: Hardcoded real-time limiter to protect hardware and hearing.
- **Profile Management**: SQLite-backed preset system with gapless A/B switching.
- **Minimal Footprint**: Runs as a macOS menu bar utility for quick access.

## Getting Started

### Prerequisites

- macOS 12.0 or later
- CMake 3.22+
- (Optional) [BlackHole](https://github.com/ExistentialAudio/BlackHole) or a similar virtual audio loopback driver for system-wide processing.

### Building from Source

```bash
# Clone the repository
git clone https://github.com/ronmkr/Equinox.git
cd Equinox

# Configure and build
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Usage

1. **Audio Routing**: To process system-wide audio, set your macOS Output device to a virtual bridge (e.g., BlackHole) and configure Equinox to use that bridge as its input.
2. **Menu Bar**: Access quick profiles, bypass toggles, and A/B comparison directly from the menu bar icon.
3. **Equalizer**: Open the main interface to customize the 31-band EQ or host external plugins.

## Architecture

Equinox is built on the JUCE 8 framework and utilizes a graph-based processing architecture:
- **Audio Engine**: Handles device I/O and graph orchestration.
- **DSP Chain**: Modular processors for EQ, Crossfeed, Loudness, and Convolution.
- **Safety Layer**: Final-stage peak limiting for hardware protection.

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on how to get started and our development standards.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
