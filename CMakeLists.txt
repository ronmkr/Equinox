cmake_minimum_required(VERSION 3.22)

project(Equinox VERSION 0.1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch JUCE via FetchContent
include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG origin/master # Or a specific version like 8.0.0
)
FetchContent_MakeAvailable(juce)

# Create the executable
juce_add_gui_app(Equinox
    PRODUCT_NAME "Equinox"
    COMPANY_NAME "EquinoxAudio"
    BUNDLE_ID "com.equinox.app"
    IS_CONTENT_SHARING_ALLOWED TRUE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_SYNTH FALSE
    NEEDS_CONSOLE_OUTPUT TRUE
    PLIST_TO_MERGE "<plist version=\"1.0\"><dict><key>LSUIElement</key><string>1</string></dict></plist>"
)

# Add source files
target_sources(Equinox PRIVATE
    src/Main.cpp
    src/dsp/FilterProcessor.cpp
    src/dsp/FilterProcessor.h
    src/dsp/AudioEngine.cpp
    src/dsp/AudioEngine.h
    src/dsp/InternalEqProcessor.h
    src/dsp/LimiterProcessor.h
    src/dsp/ProfileManager.cpp
    src/dsp/ProfileManager.h
    src/ui/MainComponent.cpp
    src/ui/MainComponent.h
    src/ui/MenuBarIcon.cpp
    src/ui/MenuBarIcon.h
    src/ui/GraphicEqComponent.cpp
    src/ui/GraphicEqComponent.h
    src/ui/ParametricCurveComponent.cpp
    src/ui/ParametricCurveComponent.h
)

# Link JUCE modules
target_link_libraries(Equinox PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
)

# Enable subnormal number handling and standard JUCE defines
juce_generate_juce_header(Equinox)

# Speed up builds with Precompiled Headers
target_precompile_headers(Equinox PRIVATE
    <juce_audio_basics/juce_audio_basics.h>
    <juce_audio_devices/juce_audio_devices.h>
    <juce_audio_formats/juce_audio_formats.h>
    <juce_audio_processors/juce_audio_processors.h>
    <juce_audio_utils/juce_audio_utils.h>
    <juce_core/juce_core.h>
    <juce_dsp/juce_dsp.h>
    <juce_gui_basics/juce_gui_basics.h>
    <juce_gui_extra/juce_gui_extra.h>
)

target_compile_definitions(Equinox PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)
