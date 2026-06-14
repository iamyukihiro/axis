#include "AxisParameters.h"

namespace {

juce::String formatDecibelValue(float value) { return juce::String(value, 1) + " dB"; }

} // namespace

namespace axis::domain {

const juce::String &parameterIdToString(ParameterId id) {
    static const juce::String input{"input"};
    static const juce::String center{"center"};
    static const juce::String sideGain{"sideGain"};
    static const juce::String density{"density"};
    static const juce::String width{"width"};
    static const juce::String output{"output"};
    static const juce::String autoGain{"autoGain"};
    static const juce::String bypass{"bypass"};

    switch (id) {
    case ParameterId::input:
        return input;
    case ParameterId::center:
        return center;
    case ParameterId::sideGain:
        return sideGain;
    case ParameterId::density:
        return density;
    case ParameterId::width:
        return width;
    case ParameterId::output:
        return output;
    case ParameterId::autoGain:
        return autoGain;
    case ParameterId::bypass:
        return bypass;
    }

    jassertfalse;
    return input;
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        parameterIdToString(ParameterId::input), "Input",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.01f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB").withStringFromValueFunction(
            [](float value, int) { return formatDecibelValue(value); })));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        parameterIdToString(ParameterId::center), "Mid",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.01f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB").withStringFromValueFunction(
            [](float value, int) { return formatDecibelValue(value); })));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        parameterIdToString(ParameterId::sideGain), "Side Gain",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.01f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB").withStringFromValueFunction(
            [](float value, int) { return formatDecibelValue(value); })));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        parameterIdToString(ParameterId::density), "Side Density",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        parameterIdToString(ParameterId::width), "Width",
        juce::NormalisableRange<float>(0.0f, 200.0f, 0.01f), 100.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        parameterIdToString(ParameterId::output), "Output",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.01f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB").withStringFromValueFunction(
            [](float value, int) { return formatDecibelValue(value); })));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        parameterIdToString(ParameterId::autoGain), "Auto Gain", true));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        parameterIdToString(ParameterId::bypass), "Bypass", false));

    return {params.begin(), params.end()};
}

} // namespace axis::domain
