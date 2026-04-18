#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "FilterProcessor.h"

namespace equinox
{

/**
 * @class InternalEqProcessor
 * @brief An AudioProcessor wrapper for FilterProcessor to allow use in AudioProcessorGraph.
 */
class InternalEqProcessor : public juce::AudioProcessor
{
public:
    InternalEqProcessor(FilterProcessor& fp) : filterProcessor(fp) {}

    const juce::String getName() const override { return "Equinox EQ"; }

    void prepareToPlay(double sampleRate, int samplesPerBlock) override
    {
        filterProcessor.prepareToPlay(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    }

    void releaseResources() override {}

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override
    {
        filterProcessor.processBlock(buffer);
    }

    // Mandatory juce::AudioProcessor overrides
    double getTailLengthSeconds() const override { return 0.0; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

private:
    FilterProcessor& filterProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InternalEqProcessor)
};

} // namespace equinox
