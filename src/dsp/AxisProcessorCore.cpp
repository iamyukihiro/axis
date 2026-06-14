#include "AxisProcessorCore.h"

#include <cmath>

namespace {

constexpr auto densityRatio = 4.0f;
constexpr auto densityKneeWidthDb = 6.0f;
constexpr auto autoTrimFloor = 0.5f;
constexpr auto densitySaturationDrive = 3.5f;
constexpr auto densitySaturationMakeup = 0.85f;

}

namespace axis::dsp {

void ProcessorCore::prepare(double sampleRate) {
    currentSampleRate = sampleRate;
    detectorState = 0.0f;
    compressorGain = 1.0f;
    autoTrimGain = 1.0f;
    clearMeters();
    updateTiming();
}

void ProcessorCore::process(juce::AudioBuffer<float> &buffer, const ParameterSnapshot &parameters) {
    if (buffer.getNumChannels() < 2) {
        clearMeters();
        return;
    }

    const auto inputGain = dbToLinear(parameters.inputDb);
    const auto centerGain = dbToLinear(parameters.centerDb);
    const auto sideGain = dbToLinear(parameters.sideGainDb);
    const auto densityAmount = juce::jlimit(0.0f, 1.0f, parameters.densityPercent * 0.01f);
    const auto width = parameters.widthPercent * 0.01f;
    const auto outputGain = dbToLinear(parameters.outputDb);

    auto *left = buffer.getWritePointer(0);
    auto *right = buffer.getWritePointer(1);
    auto blockInputPeakLeft = 0.0f;
    auto blockInputPeakRight = 0.0f;
    auto blockPeakLeft = 0.0f;
    auto blockPeakRight = 0.0f;

    for (auto sample = 0; sample < buffer.getNumSamples(); ++sample) {
        const auto inputLeft = left[sample];
        const auto inputRight = right[sample];
        const auto stagedLeft = inputLeft * inputGain;
        const auto stagedRight = inputRight * inputGain;
        blockInputPeakLeft = juce::jmax(blockInputPeakLeft, std::abs(stagedLeft));
        blockInputPeakRight = juce::jmax(blockInputPeakRight, std::abs(stagedRight));

        if (parameters.bypassEnabled) {
            blockPeakLeft = juce::jmax(blockPeakLeft, std::abs(inputLeft));
            blockPeakRight = juce::jmax(blockPeakRight, std::abs(inputRight));
            continue;
        }

        auto mid = 0.5f * (stagedLeft + stagedRight);
        auto side = 0.5f * (stagedLeft - stagedRight);

        mid *= centerGain;
        side *= sideGain;
        side = applyDensity(side, densityAmount);
        side *= width;

        auto outLeft = mid + side;
        auto outRight = mid - side;

        const auto trim = parameters.autoGainEnabled
                              ? applyAutoTrim(stagedLeft, stagedRight, outLeft, outRight)
                              : 1.0f;
        outLeft *= trim * outputGain;
        outRight *= trim * outputGain;

        if (parameters.softClipEnabled) {
            outLeft = std::tanh(outLeft);
            outRight = std::tanh(outRight);
        }

        left[sample] = outLeft;
        right[sample] = outRight;
        blockPeakLeft = juce::jmax(blockPeakLeft, std::abs(outLeft));
        blockPeakRight = juce::jmax(blockPeakRight, std::abs(outRight));
    }

    meterState.inputPeakLeft = blockInputPeakLeft;
    meterState.inputPeakRight = blockInputPeakRight;
    meterState.outputPeakLeft = blockPeakLeft;
    meterState.outputPeakRight = blockPeakRight;
}

const MeterState &ProcessorCore::getMeterState() const noexcept { return meterState; }

float ProcessorCore::dbToLinear(float dbValue) noexcept {
    return juce::Decibels::decibelsToGain(dbValue, -24.0f);
}

float ProcessorCore::gainToDb(float gain) noexcept {
    return juce::Decibels::gainToDecibels(juce::jmax(gain, 0.00001f), -100.0f);
}

float ProcessorCore::coefficientForTimeMs(float timeMs, double sampleRate) noexcept {
    return std::exp(-1.0f / (0.001f * timeMs * static_cast<float>(sampleRate)));
}

float ProcessorCore::applyDensity(float sideSample, float densityAmount) noexcept {
    if (densityAmount <= 0.0f)
        return sideSample;

    const auto sidePower = sideSample * sideSample;
    const auto detectorCoeff =
        sidePower > detectorState ? detectorAttackCoeff : detectorReleaseCoeff;
    detectorState = detectorCoeff * detectorState + (1.0f - detectorCoeff) * sidePower;

    const auto rmsDb = gainToDb(std::sqrt(detectorState + 1.0e-9f));
    const auto thresholdDb = juce::jlimit(-60.0f, 6.0f, rmsDb - 6.0f);
    const auto inputDb = gainToDb(std::abs(sideSample));
    const auto kneeStartDb = thresholdDb - densityKneeWidthDb * 0.5f;
    const auto kneeEndDb = thresholdDb + densityKneeWidthDb * 0.5f;

    auto gainReductionDb = 0.0f;

    if (inputDb > kneeEndDb) {
        gainReductionDb = (1.0f - (1.0f / densityRatio)) * (inputDb - thresholdDb);
    } else if (inputDb > kneeStartDb) {
        const auto x = inputDb - kneeStartDb;
        gainReductionDb = (1.0f - (1.0f / densityRatio)) * (x * x) / (2.0f * densityKneeWidthDb);
    }

    const auto targetGain = dbToLinear(-gainReductionDb);
    const auto gainCoeff = targetGain < compressorGain ? gainAttackCoeff : gainReleaseCoeff;
    compressorGain = gainCoeff * compressorGain + (1.0f - gainCoeff) * targetGain;

    const auto compressedSide = sideSample * compressorGain;
    const auto saturatedSide =
        std::tanh(compressedSide * (1.0f + (densityAmount * densitySaturationDrive))) *
        densitySaturationMakeup;
    const auto shapedSide = juce::jmap(densityAmount, compressedSide, saturatedSide);
    return juce::jmap(densityAmount, sideSample, shapedSide);
}

float ProcessorCore::applyAutoTrim(float inputLeft, float inputRight, float outputLeft,
                                   float outputRight) noexcept {
    const auto inputEnergy = 0.5f * ((inputLeft * inputLeft) + (inputRight * inputRight));
    const auto outputEnergy = 0.5f * ((outputLeft * outputLeft) + (outputRight * outputRight));
    auto targetTrim = 1.0f;

    if (outputEnergy > inputEnergy + 1.0e-6f)
        targetTrim = std::sqrt((inputEnergy + 1.0e-6f) / (outputEnergy + 1.0e-6f));

    targetTrim = juce::jlimit(autoTrimFloor, 1.0f, targetTrim);

    const auto trimCoeff = targetTrim < autoTrimGain ? autoTrimAttackCoeff : autoTrimReleaseCoeff;
    autoTrimGain = trimCoeff * autoTrimGain + (1.0f - trimCoeff) * targetTrim;
    return autoTrimGain;
}

void ProcessorCore::updateTiming() {
    detectorAttackCoeff = coefficientForTimeMs(10.0f, currentSampleRate);
    detectorReleaseCoeff = coefficientForTimeMs(100.0f, currentSampleRate);
    gainAttackCoeff = coefficientForTimeMs(10.0f, currentSampleRate);
    gainReleaseCoeff = coefficientForTimeMs(100.0f, currentSampleRate);
    autoTrimAttackCoeff = coefficientForTimeMs(15.0f, currentSampleRate);
    autoTrimReleaseCoeff = coefficientForTimeMs(120.0f, currentSampleRate);
}

void ProcessorCore::clearMeters() noexcept { meterState = {}; }

}
