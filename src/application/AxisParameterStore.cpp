#include "AxisParameterStore.h"

namespace axis::application {

namespace {

std::atomic<float> *bindParameter(juce::AudioProcessorValueTreeState &state,
                                  axis::domain::ParameterId id) {
    return state.getRawParameterValue(juce::String(axis::domain::parameterKey(id).data()));
}

}

void AxisParameterStore::bind(juce::AudioProcessorValueTreeState &state) {
    inputParam = bindParameter(state, axis::domain::ParameterId::input);
    centerParam = bindParameter(state, axis::domain::ParameterId::center);
    sideGainParam = bindParameter(state, axis::domain::ParameterId::sideGain);
    densityParam = bindParameter(state, axis::domain::ParameterId::density);
    sideSparkParam = bindParameter(state, axis::domain::ParameterId::sideSpark);
    sparkSendParam = bindParameter(state, axis::domain::ParameterId::sparkSend);
    sparkGainParam = bindParameter(state, axis::domain::ParameterId::sparkGain);
    sparkDuckParam = bindParameter(state, axis::domain::ParameterId::sparkDuck);
    sparkThresholdParam = bindParameter(state, axis::domain::ParameterId::sparkThreshold);
    widthParam = bindParameter(state, axis::domain::ParameterId::width);
    sparkPitchParam = bindParameter(state, axis::domain::ParameterId::sparkPitch);
    outputParam = bindParameter(state, axis::domain::ParameterId::output);
    autoGainParam = bindParameter(state, axis::domain::ParameterId::autoGain);
    softClipParam = bindParameter(state, axis::domain::ParameterId::softClip);
    sparkSoloParam = bindParameter(state, axis::domain::ParameterId::sparkSolo);
    bypassParam = bindParameter(state, axis::domain::ParameterId::bypass);
}

axis::dsp::ParameterSnapshot AxisParameterStore::snapshot() const noexcept {
    return {inputParam->load(),
            centerParam->load(),
            sideGainParam->load(),
            densityParam->load(),
            sideSparkParam->load(),
            sparkSendParam->load(),
            sparkGainParam->load(),
            sparkDuckParam->load(),
            sparkThresholdParam->load(),
            widthParam->load(),
            sparkPitchParam->load(),
            outputParam->load(),
            autoGainParam->load() >= 0.5f,
            softClipParam->load() >= 0.5f,
            sparkSoloParam->load() >= 0.5f,
            bypassParam->load() >= 0.5f};
}

void AxisParameterStore::resetToDefault(juce::AudioProcessor &processor) const {
    for (auto *parameter : processor.getParameters()) {
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->getDefaultValue());
        parameter->endChangeGesture();
    }
}

}
