#pragma once

#include <JuceHeader.h>

class AxisCenterAudioProcessor final : public juce::AudioProcessor
{
public:
    AxisCenterAudioProcessor();
    ~AxisCenterAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
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
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    void resetParametersToDefault();
    float getOutputPeakLeft() const noexcept;
    float getOutputPeakRight() const noexcept;

    juce::AudioProcessorValueTreeState apvts;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    enum class ParameterId
    {
        center,
        density,
        width,
        output
    };

    static const juce::String parameterIdToString(ParameterId id);
    static float dbToLinear(float dbValue) noexcept;
    static float gainToDb(float gain) noexcept;
    static float coefficientForTimeMs(float timeMs, double sampleRate) noexcept;
    float applyDensity(float sideSample, float densityAmount) noexcept;
    float applyAutoTrim(float inputLeft, float inputRight, float outputLeft, float outputRight) noexcept;
    void updateTiming();

    std::atomic<float>* centerParam = nullptr;
    std::atomic<float>* densityParam = nullptr;
    std::atomic<float>* widthParam = nullptr;
    std::atomic<float>* outputParam = nullptr;
    std::atomic<float> outputPeakLeft { 0.0f };
    std::atomic<float> outputPeakRight { 0.0f };
    double currentSampleRate = 44100.0;
    float detectorState = 0.0f;
    float compressorGain = 1.0f;
    float autoTrimGain = 1.0f;
    float detectorAttackCoeff = 0.0f;
    float detectorReleaseCoeff = 0.0f;
    float gainAttackCoeff = 0.0f;
    float gainReleaseCoeff = 0.0f;
    float autoTrimAttackCoeff = 0.0f;
    float autoTrimReleaseCoeff = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AxisCenterAudioProcessor)
};
