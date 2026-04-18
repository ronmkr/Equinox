# Debugging Equinox

This guide covers common strategies for diagnosing stability and performance issues in the Equinox DSP engine and macOS Menu Bar wrapper.

## 1. Diagnosing Crashes (Segmentation Faults)
Because Equinox runs as a background agent (`LSUIElement`), it may crash silently.

### Terminal Execution
Always run the executable directly from the terminal to see `stdout` and `stderr`:
```bash
./build/Equinox_artefacts/Equinox.app/Contents/MacOS/Equinox
```
If it crashes, the terminal will report `Segmentation fault: 11`.

### Isolation Strategy
If a crash occurs during startup:
1.  **Disable the Menu Bar Icon:** Comment out `m_menuBarIcon` in `Main.cpp`.
2.  **Disable the Audio Engine:** Comment out `m_audioEngine.initialize()`.
3.  **Disable DSP Nodes:** Comment out nodes one-by-one in `AudioEngine::setupGraph()`.

## 2. Common JUCE 8 Gotchas
- **Channel Count Safety:** `AudioProcessorGraph` may call `prepareToPlay` or `processBlock` with 0 channels during routing changes. **Always check `getTotalNumInputChannels() > 0` before processing.**
- **Initialization Order:** Never call `AudioProcessorGraph::prepareToPlay` before the `AudioDeviceManager` has successfully initialized a device.
- **Message Thread vs. Audio Thread:** Ensure UI controls (like `ComboBox`) are never accessed directly from `processBlock`.

## 3. Real-Time Safety Verification
- **Memory Allocation:** No `new` or `std::vector::push_back` in `processBlock`.
- **System Calls:** No `printf` or `juce::Logger` in the audio callback.
- **Tools:** Use `AUVal` (Audio Unit Validation Tool) to check plugin compatibility if hosted plugins cause instability.

## 4. Useful Commands
- **Kill Ghost Processes:** `pkill Equinox`
- **Check if Running:** `ps aux | grep Equinox`
- **View Plist:** `plutil -p build/Equinox_artefacts/Equinox.app/Contents/Info.plist`
