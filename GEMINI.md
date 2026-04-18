# Equinox: macOS Audiophile Equalizer - Development Mandates

## 1. Real-Time Audio Thread Safety (CRITICAL)
All code within `processBlock` or called by the audio callback MUST be real-time safe.
- **NO Memory Allocation:** Do not use `new`, `malloc`, `std::vector::push_back`, or JUCE `Array`/`OwnedArray` resizing.
- **NO Blocking:** No file I/O, no `std::cout`, no network requests.
- **NO Mutexes:** Use `std::atomic`, `juce::AbstractFifo`, or lock-free parameters.
- **Denormals:** Always call `juce::FloatVectorOperations::disableDenormalisedNumberSupport()` in `prepareToPlay`.

## 2. DSP & Graph Architecture
- **Graph-Based Processing:** Use `juce::AudioProcessorGraph` for all signal routing.
- **Modification Safety:** Always call `m_mainGraph->suspendProcessing(true)` before modifying graph connections or nodes.
- **Safety Limiter:** The `LimiterProcessor` MUST remain at the end of the signal chain to protect hardware from gain spikes in hosted plugins.

## 3. macOS & JUCE Specifics
- **JUCE 8:** Target JUCE 8 (C++20) for all new development.
- **Plugin Hosting:** Support VST3 and AU formats on macOS. Use `juce::addDefaultFormatsToManager()` for initialization.
- **Adhere to JUCE coding standards** (CamelCase for classes/methods).

## 4. Modern C++ Best Practices
- **Naming:** Use `m_` prefix for all private member variables (e.g., `m_sampleRate`).
- **Const Correctness:** Use `const` and `[[nodiscard]]` for all non-mutating methods.
- **RAII:** Use `std::unique_ptr` and `std::make_unique` for all heap-allocated resources.
- **Value Semantics:** Return small objects and `juce::PluginDescription` by value where appropriate to avoid dangling references.
