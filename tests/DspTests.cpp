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
    float mag = filterProcessor.getMagnitudeForFrequency(1000.0f, 44100.0);
    EXPECT_NEAR(mag, 1.0f, 0.001f);
}

TEST_F(DspTest, FilterMagnitudePeak)
{
    filterProcessor.updateBandGain(0, 1000.0f, 6.0f, 1.0f);
    float mag = filterProcessor.getMagnitudeForFrequency(1000.0f, 44100.0);
    EXPECT_NEAR(mag, 1.995f, 0.01f); 
}

TEST_F(DspTest, FunctionalGainChange)
{
    // Test that processBlock actually changes samples (Regression for slider bug)
    juce::AudioBuffer<float> buffer(2, 512);
    for(int i=0; i<512; ++i) 
        buffer.setSample(0, i, std::sin(2.0f * juce::MathConstants<float>::pi * 1000.0f * i / 44100.0f));

    float originalSample = buffer.getSample(0, 256);
    
    // Set massive boost
    filterProcessor.updateBandGain(0, 1000.0f, 12.0f, 1.0f);
    filterProcessor.processBlock(buffer);
    
    float processedSample = buffer.getSample(0, 256);
    
    // Processed should be significantly different from original
    EXPECT_NE(originalSample, processedSample);
    EXPECT_GT(std::abs(processedSample), std::abs(originalSample));
}

TEST_F(DspTest, AutoPreampCalculation)
{
    filterProcessor.updateBandGain(5, 500.0f, 10.0f, 1.0f);
    float maxGain = filterProcessor.calculateMaxGain();
    EXPECT_NEAR(maxGain, 10.0f, 0.1f);
}

TEST_F(DspTest, GlobalBypass)
{
    filterProcessor.updateBandGain(0, 1000.0f, 12.0f, 1.0f);
    filterProcessor.setBypassed(true);

    juce::AudioBuffer<float> buffer(2, 512);
    for(int i=0; i<512; ++i) 
        buffer.setSample(0, i, std::sin(2.0f * juce::MathConstants<float>::pi * 1000.0f * i / 44100.0f));

    float originalRMS = buffer.getRMSLevel(0, 0, 512);
    filterProcessor.processBlock(buffer);
    EXPECT_NEAR(buffer.getRMSLevel(0, 0, 512), originalRMS, 0.001f);
    
    filterProcessor.setBypassed(false);
    filterProcessor.processBlock(buffer);
    EXPECT_GT(buffer.getRMSLevel(0, 0, 512), originalRMS * 1.5f);
}

TEST_F(DspTest, CrossfeedBypass)
{
    CrossfeedProcessor cf;
    cf.prepareToPlay(44100.0, 128);
    cf.setEnabled(false);

    juce::AudioBuffer<float> buffer(2, 128);
    buffer.clear();
    buffer.setSample(0, 0, 1.0f);

    juce::MidiBuffer midi;
    cf.processBlock(buffer, midi);

    EXPECT_EQ(buffer.getSample(0, 0), 1.0f);
    EXPECT_EQ(buffer.getSample(1, 0), 0.0f);
}

TEST_F(DspTest, ZeroChannelSafety)
{
    juce::AudioBuffer<float> emptyBuffer(0, 128);
    juce::MidiBuffer midi;
    filterProcessor.processBlock(emptyBuffer);
    
    CrossfeedProcessor cf;
    cf.prepareToPlay(44100.0, 128);
    cf.processBlock(emptyBuffer, midi);
}

TEST_F(DspTest, RapidPrepareToPlay)
{
    // ...
}

TEST_F(DspTest, CompressorFunctionality)
{
    // Basic compressor test
    juce::dsp::Compressor<float> comp;
    juce::dsp::ProcessSpec spec { 44100.0, 512, 2 };
    comp.prepare(spec);
    
    comp.setThreshold(-20.0f);
    comp.setRatio(4.0f);
    comp.setAttack(1.0f);
    comp.setRelease(100.0f);

    juce::AudioBuffer<float> buffer(2, 512);
    for(int i=0; i<512; ++i) buffer.setSample(0, i, 1.0f); // Full scale signal (0dB)

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    comp.process(context);

    // Compressed signal should be significantly quieter than 1.0f
    EXPECT_LT(buffer.getSample(0, 511), 0.5f);
}

} // namespace equinox
