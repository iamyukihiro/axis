#include "JuceParameterLayout.h"

#include "domain/AxisParameterModel.h"

namespace {

juce::String formatDecibelValue(float value) { return juce::String(value, 1) + " dB"; }

std::unique_ptr<juce::RangedAudioParameter>
createParameter(const axis::domain::ParameterSpec &spec) {
    const auto parameterId = juce::ParameterID{juce::String(spec.key.data()), 1};
    const auto parameterName = juce::String(spec.name.data());

    if (spec.kind == axis::domain::ParameterKind::boolValue)
        return std::make_unique<juce::AudioParameterBool>(parameterId, parameterName,
                                                          spec.defaultValue >= 0.5f);

    auto attributes =
        juce::AudioParameterFloatAttributes().withLabel(juce::String(spec.unitLabel.data()));

    if (spec.unitLabel == "dB") {
        attributes = attributes.withStringFromValueFunction(
            [](float value, int) { return formatDecibelValue(value); });
    }

    return std::make_unique<juce::AudioParameterFloat>(
        parameterId, parameterName,
        juce::NormalisableRange<float>(spec.range.min, spec.range.max, spec.range.step),
        spec.defaultValue, attributes);
}

}

namespace axis::infrastructure {

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.reserve(axis::domain::parameterSpecs().size());

    for (const auto &spec : axis::domain::parameterSpecs())
        params.push_back(createParameter(spec));

    return {params.begin(), params.end()};
}

}
