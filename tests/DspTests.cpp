#include <gtest/gtest.h>
#include "../src/dsp/FilterProcessor.h"
#include "../src/dsp/CrossfeedProcessor.h"

namespace equinox
{

class DspTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        filterProcessor.prepareToPlay(44100.0, 512, 2);
    }

    FilterProcessor filterProcessor;
};

TEST_F(DspTest, FilterMagnitudeIdentity)
{
    // With no gains set, magnitude at any frequency should be 1.0 (0dB)
    float mag = filterProcessor.getMagnitudeForFrequency(1000.0f, 44100.0);
    EXPECT_NEAR(mag, 1.0f, 0.001f);
}

TEST_F(DspTest, FilterMagnitudePeak)
{
    // Set a 6dB peak at 1kHz
    filterProcessor.updateBandGain(0, 1000.0f, 6.0f, 1.0f);
    
    float mag = filterProcessor.getMagnitudeForFrequency(1000.0f, 44100.0);
    // 6dB gain is approx factor of 2.0
    EXPECT_NEAR(mag, 1.995f, 0.01f); 
}

TEST_F(DspTest, AutoPreampCalculation)
{
    // Set a 10dB peak
    filterProcessor.updateBandGain(5, 500.0f, 10.0f, 1.0f);
    
    float maxGain = filterProcessor.calculateMaxGain();
    EXPECT_NEAR(maxGain, 10.0f, 0.1f);
}

TEST_F(DspTest, GlobalBypass)
{
    filterProcessor.updateBandGain(0, 1000.0f, 12.0f, 1.0f);
    filterProcessor.setBypassed(true);

    juce::AudioBuffer<float> buffer(2, 512);
    // Fill with 1kHz Sine
    for(int i=0; i<512; ++i) 
        buffer.setSample(0, i, std::sin(2.0f * juce::MathConstants<float>::pi * 1000.0f * i / 44100.0f));

    float originalRMS = buffer.getRMSLevel(0, 0, 512);
    filterProcessor.processBlock(buffer);
    EXPECT_NEAR(buffer.getRMSLevel(0, 0, 512), originalRMS, 0.001f);
    
    filterProcessor.setBypassed(false);
    filterProcessor.processBlock(buffer);
    // With 12dB boost, RMS should be significantly higher
    EXPECT_GT(buffer.getRMSLevel(0, 0, 512), originalRMS * 1.5f);
}

TEST_F(DspTest, CrossfeedBypass)
{
    CrossfeedProcessor cf;
    cf.prepareToPlay(44100.0, 128);
    cf.setEnabled(false);

    juce::AudioBuffer<float> buffer(2, 128);
    buffer.clear();
    buffer.setSample(0, 0, 1.0f); // Impulse in left

    juce::MidiBuffer midi;
    cf.processBlock(buffer, midi);

    // When disabled, signal should remain unchanged
    EXPECT_EQ(buffer.getSample(0, 0), 1.0f);
    EXPECT_EQ(buffer.getSample(1, 0), 0.0f);
}

} // namespace equinox
