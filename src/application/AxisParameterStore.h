#pragma once

#include <JuceHeader.h>

#include "domain/AxisParameterModel.h"
#include "dsp/AxisProcessorCore.h"

namespace axis::application {

class AxisParameterStore {
  public:
    void bind(juce::AudioProcessorValueTreeState &state);
    axis::dsp::ParameterSnapshot snapshot() const noexcept;
    void resetToDefault(juce::AudioProcessor &processor) const;

  private:
    std::atomic<float> *inputParam = nullptr;
    std::atomic<float> *centerParam = nullptr;
    std::atomic<float> *sideGainParam = nullptr;
    std::atomic<float> *densityParam = nullptr;
    std::atomic<float> *widthParam = nullptr;
    std::atomic<float> *outputParam = nullptr;
    std::atomic<float> *autoGainParam = nullptr;
    std::atomic<float> *bypassParam = nullptr;
};

}
