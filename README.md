# Equinox

**Equinox** is a system-wide, low-latency audio equalization and processing application for macOS designed for audiophiles.

## Core Features
- **31-band Graphic and Parametric EQ**: High-precision filters using JUCE `dsp` module.
- **AutoEQ/Squiglink curve import**: Direct parsing of standard AutoEQ strings.
- **VST3/AU Plugin Hosting**: Integrated `AudioProcessorGraph` for serial plugin chains.
- **Safety Limiter**: Real-time `dsp::Limiter` at the end of the signal path to prevent output clipping.
- **Profile Management**: Fast profile switching via SQLite with gapless A/B comparison.
- **macOS Menu Bar App**: Runs silently in the background with quick access to common tasks.

## 🚀 How to Use

### 1. Menu Bar Utility
After launching Equinox, you will see a white circle icon in your macOS menu bar.
*   **Left/Right Click the Icon:** Opens the quick-access menu.
*   **Switching Profiles:** Hover over "Profiles" to instantly select a saved preset.
*   **Toggle A/B:** Quickly switch between two profile states to compare sound signatures.
*   **Bypass:** Instantly disable all processing to hear the original signal.

### 2. Deep Customization
Click **"Open Equalizer..."** from the menu to bring up the main interface:
*   **Equalizer Tab:** Drag the 31 sliders or double-click to reset. The parametric visualizer shows the combined curve in real-time.
*   **Plugins Tab:** 
    1.  Click **"Scan for Plugins"** to find VST3 and AU plugins on your Mac.
    2.  Select a plugin on the left and click **"Add Selected"**.
    3.  Plugins are processed in series: `Input -> EQ -> Plugin 1 -> Plugin 2 -> Limiter -> Output`.
*   **Settings Tab:** Select your virtual input device (loopback) and your physical output hardware.

### 3. Importing AutoEQ
Paste a standard AutoEQ string (e.g., from [AutoEQ.app](https://autoeq.app)) into the "Import" field to automatically configure all 31 bands.

---

## 🛠 Architecture
Equinox uses a modular graph-based architecture:
1.  **Audio Input**: macOS CoreAudio loopback capture.
2.  **EQ Engine**: 31-band biquad cascade (`FilterProcessor`).
3.  **Plugin Chain**: Serially hosted external VST3/AU plugins.
4.  **Limiter**: Final stage protection (`LimiterProcessor`).
5.  **Audio Output**: Physical hardware output.

## ⚙️ Building the Project

### Prerequisites
- macOS 12.0 or later.
- CMake 3.22+.

### Build Instructions
```bash
mkdir build && cd build
cmake ..
cmake --build . --parallel $(sysctl -n hw.ncpu)
```

### ⚡ Why are builds slow?
JUCE is a massive framework. We've optimized the project for JUCE 8. To further improve speed:
*   **Avoid clean builds:** Only use `rm -rf build` when absolutely necessary.
*   **Use Parallel Building:** Always use the `--parallel` flag as shown above.
*   **Install Ninja (Recommended):** `brew install ninja`. Then build with `cmake .. -G Ninja`.

## 🛡 Security & Safety
Equinox is built with real-time audio safety in mind. All DSP parameter changes are lock-free and atomic. A safety limiter is hardcoded at the end of the signal chain to protect your ears and hardware from unexpected gain spikes.

## License
MIT License.
