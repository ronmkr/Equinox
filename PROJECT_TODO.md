# Equinox Project Roadmap

- [x] **Phase 1: Project Scaffolding & DSP Engine**
    - [x] Setup CMake and JUCE modules.
    - [x] Implement `FilterProcessor` (AutoEQ parsing + 31 Biquad cascade).
- [x] **Phase 2: macOS System Routing**
    - [x] CoreAudio loopback logic (Virtual Input -> Physical Output).
- [x] **Phase 3: The Dual-Mode UI Sync**
    - [x] 31-band Graphic Slider UI.
    - [x] Parametric Visual UI with curve drawing.
- [x] **Phase 4: Profile DB & A/B Toggle**
    - [x] SQLite schema for profiles (XML persistent store).
    - [x] Gapless A/B pointer swapping.
- [x] **Phase 5: VST/AU Hosting**
    - [x] `AudioProcessorGraph` integration for external plugins.
    - [x] Plugin scanning and hosting infrastructure.
    - [x] Basic UI for plugin management.
- [x] **Phase 6: Menu Bar App Wrapping**
    - [x] Refactor to macOS Menu Bar (Status Item).
    - [x] Toggleable utility window with "Always on Top".
    - [x] Native quick-access menu for profiles and toggles.
- [x] **Phase 7: Headphone DSP & Safety**
    - [x] Crossfeed algorithm (Linkwitz-style implementation).
    - [x] `dsp::Limiter` (Safety Limiter implemented).
    - [x] Auto-Preamp calculator.

- [x] **Phase 8: Hardware State & Dynamic Adaptation**
    - [x] Device listeners and UUID-based profile switching.
- [x] **Phase 9: Convolution & Multi-Mono**
    - [x] `dsp::Convolution` for FIR (WAV) loading.
    - [x] Independent L/R channel processing (via Stereo Convolution).
- [x] **Phase 10: Dynamic Loudness Compensation**
    - [x] ISO 226:2003 implementation linked to system volume.

## Project Status: COMPLETE
All 10 phases of the Equinox project have been implemented, refactored for modularity, and verified through both automated unit tests and real-world execution logs.
- [x] Real-time DSP safety (Limiter, Lock-free params).
- [x] VST3/AU Hosting via AudioProcessorGraph.
- [x] AutoEQ parsing and 31-band Graphic/Parametric Sync.
- [x] macOS Menu Bar integration.
- [x] Hardware-aware profile switching.
- [x] Spectrum Analyzer UI.
- [x] Dynamic Loudness & Convolution IR loading.
