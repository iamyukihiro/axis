#include <JuceHeader.h>

#include "domain/AxisParameterModel.h"
#include "infrastructure/JuceParameterLayout.h"

namespace {

class TestProcessor final : public juce::AudioProcessor {
  public:
    TestProcessor()
        : AudioProcessor(BusesProperties()
                             .withInput("Input", juce::AudioChannelSet::stereo(), true)
                             .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
          apvts(*this, nullptr, "PARAMETERS", axis::infrastructure::createParameterLayout()) {}

    const juce::String getName() const override { return "AxisTestProcessor"; }
    void prepareToPlay(double, int) override {}
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout &) const override { return true; }
    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override {}
    juce::AudioProcessorEditor *createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String &) override {}
    void getStateInformation(juce::MemoryBlock &) override {}
    void setStateInformation(const void *, int) override {}

    juce::AudioProcessorValueTreeState apvts;
};

class AxisParametersTests final : public juce::UnitTest {
  public:
    AxisParametersTests() : juce::UnitTest("AxisParameters", "axis") {}

    void runTest() override {
        TestProcessor processor;

        beginTest("parameter id が期待どおりに並ぶ");
        juce::StringArray expectedIds;
        for (const auto &spec : axis::domain::parameterSpecs())
            expectedIds.add(juce::String(spec.key.data()));
        expectEquals(processor.getParameters().size(), static_cast<int>(expectedIds.size()));

        for (int index = 0; index < expectedIds.size(); ++index)
            expect(processor.apvts.getParameter(expectedIds[index]) ==
                   processor.getParameters()[index]);

        beginTest("default value が期待どおり");
        expectWithinAbsoluteError(processor.apvts.getRawParameterValue("input")->load(), 0.0f,
                                  0.0001f);
        expectWithinAbsoluteError(processor.apvts.getRawParameterValue("center")->load(), 0.0f,
                                  0.0001f);
        expectWithinAbsoluteError(processor.apvts.getRawParameterValue("sideGain")->load(), 0.0f,
                                  0.0001f);
        expectWithinAbsoluteError(processor.apvts.getRawParameterValue("density")->load(), 0.0f,
                                  0.0001f);
        expectWithinAbsoluteError(processor.apvts.getRawParameterValue("sideSpark")->load(), 0.0f,
                                  0.0001f);
        expectWithinAbsoluteError(processor.apvts.getRawParameterValue("sparkSend")->load(), -9.0f,
                                  0.0001f);
        expectWithinAbsoluteError(processor.apvts.getRawParameterValue("sparkGain")->load(), 6.0f,
                                  0.0001f);
        expectWithinAbsoluteError(processor.apvts.getRawParameterValue("sparkDuck")->load(), 15.0f,
                                  0.0001f);
        expectWithinAbsoluteError(processor.apvts.getRawParameterValue("sparkThreshold")->load(),
                                  50.0f, 0.0001f);
        expectWithinAbsoluteError(processor.apvts.getRawParameterValue("width")->load(), 100.0f,
                                  0.0001f);
        expectWithinAbsoluteError(processor.apvts.getRawParameterValue("sparkPitch")->load(), 0.0f,
                                  0.0001f);
        expectWithinAbsoluteError(processor.apvts.getRawParameterValue("output")->load(), 0.0f,
                                  0.0001f);
        expectWithinAbsoluteError(processor.apvts.getRawParameterValue("autoGain")->load(), 1.0f,
                                  0.0001f);
        expectWithinAbsoluteError(processor.apvts.getRawParameterValue("softClip")->load(), 0.0f,
                                  0.0001f);
        expectWithinAbsoluteError(processor.apvts.getRawParameterValue("sparkSolo")->load(), 0.0f,
                                  0.0001f);
        expectWithinAbsoluteError(processor.apvts.getRawParameterValue("bypass")->load(), 0.0f,
                                  0.0001f);

        beginTest("代表的な range が期待どおり");
        const auto inputRange = processor.apvts.getParameterRange("input");
        expectWithinAbsoluteError(inputRange.start, -24.0f, 0.0001f);
        expectWithinAbsoluteError(inputRange.end, 12.0f, 0.0001f);

        const auto widthRange = processor.apvts.getParameterRange("width");
        expectWithinAbsoluteError(widthRange.start, 0.0f, 0.0001f);
        expectWithinAbsoluteError(widthRange.end, 200.0f, 0.0001f);

        const auto sparkRange = processor.apvts.getParameterRange("sideSpark");
        expectWithinAbsoluteError(sparkRange.start, 0.0f, 0.0001f);
        expectWithinAbsoluteError(sparkRange.end, 150.0f, 0.0001f);

        const auto sparkSendRange = processor.apvts.getParameterRange("sparkSend");
        expectWithinAbsoluteError(sparkSendRange.start, -24.0f, 0.0001f);
        expectWithinAbsoluteError(sparkSendRange.end, 24.0f, 0.0001f);

        const auto sparkGainRange = processor.apvts.getParameterRange("sparkGain");
        expectWithinAbsoluteError(sparkGainRange.start, 0.0f, 0.0001f);
        expectWithinAbsoluteError(sparkGainRange.end, 36.0f, 0.0001f);

        const auto sparkDuckRange = processor.apvts.getParameterRange("sparkDuck");
        expectWithinAbsoluteError(sparkDuckRange.start, 0.0f, 0.0001f);
        expectWithinAbsoluteError(sparkDuckRange.end, 100.0f, 0.0001f);

        const auto sparkThresholdRange = processor.apvts.getParameterRange("sparkThreshold");
        expectWithinAbsoluteError(sparkThresholdRange.start, 0.0f, 0.0001f);
        expectWithinAbsoluteError(sparkThresholdRange.end, 100.0f, 0.0001f);

        const auto sparkPitchRange = processor.apvts.getParameterRange("sparkPitch");
        expectWithinAbsoluteError(sparkPitchRange.start, -24.0f, 0.0001f);
        expectWithinAbsoluteError(sparkPitchRange.end, 24.0f, 0.0001f);
    }
};

static AxisParametersTests axisParametersTests;

}
