#pragma once

#include <JuceHeader.h>

#include "application/AxisParameterStore.h"
#include "dsp/AxisProcessorCore.h"

class AxisCenterAudioProcessor final : public juce::AudioProcessor {
  public:
    AxisCenterAudioProcessor();
    ~AxisCenterAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    juce::AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String &newName) override;

    void getStateInformation(juce::MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;
    void resetParametersToDefault();
    float getInputPeakLeft() const noexcept;
    float getInputPeakRight() const noexcept;
    float getSparkPeakLeft() const noexcept;
    float getSparkPeakRight() const noexcept;
    float getOutputPeakLeft() const noexcept;
    float getOutputPeakRight() const noexcept;

    juce::AudioProcessorValueTreeState apvts;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

  private:
    std::atomic<float> inputPeakLeft{0.0f};
    std::atomic<float> inputPeakRight{0.0f};
    std::atomic<float> sparkPeakLeft{0.0f};
    std::atomic<float> sparkPeakRight{0.0f};
    std::atomic<float> outputPeakLeft{0.0f};
    std::atomic<float> outputPeakRight{0.0f};
    axis::application::AxisParameterStore parameterStore;
    axis::dsp::ProcessorCore processorCore;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AxisCenterAudioProcessor)
};
