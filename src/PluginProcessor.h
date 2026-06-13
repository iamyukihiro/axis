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
        centerGain,
        sideGain,
        width,
        lowMonoFrequency,
        outputGain,
        softClip,
        bypass
    };

    struct OnePoleHighPass
    {
        void prepare(double newSampleRate);
        void setCutoff(float newCutoffHz);
        float processSample(float inputSample) noexcept;
        void reset() noexcept;

        double sampleRate = 44100.0;
        float cutoffHz = 0.0f;
        float alpha = 0.0f;
        float previousInput = 0.0f;
        float previousOutput = 0.0f;
    };

    static const juce::String parameterIdToString(ParameterId id);
    static float dbToLinear(float dbValue) noexcept;
    void updateFilters();

    OnePoleHighPass sideHighPass;
    std::atomic<float>* centerGainParam = nullptr;
    std::atomic<float>* sideGainParam = nullptr;
    std::atomic<float>* widthParam = nullptr;
    std::atomic<float>* lowMonoFrequencyParam = nullptr;
    std::atomic<float>* outputGainParam = nullptr;
    std::atomic<float>* softClipParam = nullptr;
    std::atomic<float>* bypassParam = nullptr;
    std::atomic<float> outputPeakLeft { 0.0f };
    std::atomic<float> outputPeakRight { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AxisCenterAudioProcessor)
};
