#pragma once

#include <JuceHeader.h>

namespace axis::domain {

enum class ParameterId { input, center, sideGain, density, width, output, autoGain, bypass };

const juce::String &parameterIdToString(ParameterId id);
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

} // namespace axis::domain
