#pragma once

#include <JuceHeader.h>

namespace axis::infrastructure {

void writeStateToMemory(juce::AudioProcessorValueTreeState &state, juce::MemoryBlock &destData);
void restoreStateFromMemory(juce::AudioProcessorValueTreeState &state, const void *data,
                            int sizeInBytes);

}
