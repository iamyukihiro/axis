#include "JucePluginState.h"

namespace axis::infrastructure {

void writeStateToMemory(juce::AudioProcessorValueTreeState &state, juce::MemoryBlock &destData) {
    if (auto xml = state.copyState().createXml())
        juce::AudioProcessor::copyXmlToBinary(*xml, destData);
}

void restoreStateFromMemory(juce::AudioProcessorValueTreeState &state, const void *data,
                            int sizeInBytes) {
    if (auto xmlState = juce::AudioProcessor::getXmlFromBinary(data, sizeInBytes))
        if (xmlState->hasTagName(state.state.getType()))
            state.replaceState(juce::ValueTree::fromXml(*xmlState));
}

}
