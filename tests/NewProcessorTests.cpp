#include <gtest/gtest.h>
#include "../src/dsp/LoudnessProcessor.h"
#include "../src/dsp/ConvolutionProcessor.h"
#include "../src/dsp/ProfileManager.h"

using namespace equinox;

class NewProcessorTest : public ::testing::Test {
protected:
    void SetUp() override {}
};

TEST_F(NewProcessorTest, LoudnessGainScaling) {
    LoudnessProcessor loudness;
    loudness.prepareToPlay(44100.0, 512);
    loudness.setEnabled(true);

    juce::AudioBuffer<float> buffer(2, 512);
    
    // Test 1: 0dB Volume (Should have near 1.0 gain at 100Hz)
    loudness.setVolumeDb(0.0f);
    buffer.clear();
    for (int i=0; i<512; ++i) buffer.setSample(0, i, 1.0f); // DC-ish for simple check
    juce::MidiBuffer midi;
    loudness.processBlock(buffer, midi);
    float gainAt0 = buffer.getSample(0, 0);

    // Test 2: -40dB Volume (Should have significantly higher gain due to boost)
    loudness.setVolumeDb(-40.0f);
    buffer.clear();
    for (int i=0; i<512; ++i) buffer.setSample(0, i, 1.0f);
    loudness.processBlock(buffer, midi);
    float gainAt40 = buffer.getSample(0, 0);

    EXPECT_GT(gainAt40, gainAt0);
}

TEST_F(NewProcessorTest, ConvolutionToggleState) {
    ConvolutionProcessor convolver;
    EXPECT_FALSE(convolver.isEnabled());
    convolver.setEnabled(true);
    EXPECT_TRUE(convolver.isEnabled());
}

TEST_F(NewProcessorTest, ProfileManagerHardwareMapping) {
    ProfileManager pm;
    juce::String testUuid = "Test-Device-123";
    juce::String testProfile = "Audiophile-HD600";
    
    pm.setProfileForDevice(testUuid, testProfile);
    auto retrieved = pm.getProfileForDevice(testUuid);
    
    EXPECT_EQ(retrieved, testProfile);
}
