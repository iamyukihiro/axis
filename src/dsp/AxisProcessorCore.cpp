#include "AxisProcessorCore.h"

#include <cmath>

namespace {

constexpr auto densityRatio = 4.0f;
constexpr auto densityKneeWidthDb = 6.0f;
constexpr auto autoTrimFloor = 0.5f;
constexpr auto densitySaturationDrive = 3.5f;
constexpr auto densitySaturationMakeup = 0.85f;
constexpr auto sparkTransientFloor = 0.006f;
constexpr auto sparkTransientSensitivity = 0.34f;
constexpr auto sparkBurstLevel = 1.75f;
constexpr auto sparkLimiterCeiling = 0.36f;
constexpr auto sparkHighPassFrequency = 1500.0f;
constexpr auto sparkLowPassFrequency = 8000.0f;
constexpr auto sparkFastTimeMs = 0.35f;
constexpr auto sparkSlowTimeMs = 12.0f;
constexpr auto sparkLengthMs = 3.0f;
constexpr auto sparkCooldownMs = 6.0f;
constexpr auto sparkDuckScale = 1.1f;
constexpr auto sparkPitchBufferMs = 35.0f;
constexpr auto sparkPitchLagMs = 12.0f;
constexpr auto sparkPitchMinLagSamples = 32.0f;

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
    const auto sparkAmount = juce::jlimit(0.0f, 1.5f, parameters.sideSparkPercent * 0.01f);
    const auto sparkSend = dbToLinear(parameters.sparkSendDb);
    const auto sparkGain = dbToLinear(parameters.sparkGainDb);
    const auto sparkDuckMaxReduction =
        juce::jlimit(0.0f, 1.0f, parameters.sparkDuckPercent * 0.01f);
    const auto sparkThresholdControl = juce::jlimit(0.0f, 100.0f, parameters.sparkThresholdPercent);
    const auto width = parameters.widthPercent * 0.01f;
    const auto sparkPitchSemitones = parameters.sparkPitchSemitones;
    const auto outputGain = dbToLinear(parameters.outputDb);

    auto *left = buffer.getWritePointer(0);
    auto *right = buffer.getWritePointer(1);
    auto blockInputPeakLeft = 0.0f;
    auto blockInputPeakRight = 0.0f;
    auto blockSparkDetectLeft = 0.0f;
    auto blockSparkDetectRight = 0.0f;
    auto blockSparkThresholdLeft = 0.0f;
    auto blockSparkThresholdRight = 0.0f;
    auto blockSparkDuckLeft = 0.0f;
    auto blockSparkDuckRight = 0.0f;
    auto blockSparkPeakLeft = 0.0f;
    auto blockSparkPeakRight = 0.0f;
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
        const auto baseSide = applyDensity(side, densityAmount);
        const auto sparkSide =
            applySideSpark(baseSide, sparkAmount, sparkSend, sparkGain, sparkThresholdControl);
        const auto sparkDuckAmount =
            juce::jlimit(0.0f, sparkDuckMaxReduction, std::abs(sparkSide) * sparkDuckScale);
        const auto duckedSide = baseSide * (1.0f - sparkDuckAmount);
        const auto drySide = duckedSide;
        const auto pitchedSparkSide = applySparkPitch(sparkSide, sparkPitchSemitones);
        const auto sparkWidthSide = pitchedSparkSide * width;
        side = duckedSide + sparkWidthSide;
        const auto sparkContributionSide = side - drySide;
        const auto sparkLeft = sparkContributionSide;
        const auto sparkRight = -sparkContributionSide;
        blockSparkPeakLeft = juce::jmax(blockSparkPeakLeft, std::abs(sparkLeft));
        blockSparkPeakRight = juce::jmax(blockSparkPeakRight, std::abs(sparkRight));
        blockSparkDetectLeft = juce::jmax(blockSparkDetectLeft, meterState.sparkDetectLeft);
        blockSparkDetectRight = juce::jmax(blockSparkDetectRight, meterState.sparkDetectRight);
        blockSparkThresholdLeft = meterState.sparkThresholdLeft;
        blockSparkThresholdRight = meterState.sparkThresholdRight;
        blockSparkDuckLeft = juce::jmax(blockSparkDuckLeft, sparkDuckAmount);
        blockSparkDuckRight = juce::jmax(blockSparkDuckRight, sparkDuckAmount);

        auto outLeft = 0.0f;
        auto outRight = 0.0f;
        if (parameters.sparkSoloEnabled) {
            outLeft = sparkLeft;
            outRight = sparkRight;
        } else {
            outLeft = mid + side;
            outRight = mid - side;
        }

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
    meterState.sparkDetectLeft = blockSparkDetectLeft;
    meterState.sparkDetectRight = blockSparkDetectRight;
    meterState.sparkThresholdLeft = blockSparkThresholdLeft;
    meterState.sparkThresholdRight = blockSparkThresholdRight;
    meterState.sparkDuckLeft = blockSparkDuckLeft;
    meterState.sparkDuckRight = blockSparkDuckRight;
    meterState.sparkPeakLeft = blockSparkPeakLeft;
    meterState.sparkPeakRight = blockSparkPeakRight;
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

float ProcessorCore::applySideSpark(float sideSample, float sparkAmount, float sparkSend,
                                    float sparkGain, float thresholdControl) noexcept {
    if (sparkAmount <= 0.0f) {
        sparkFastEnvelope = 0.0f;
        sparkSlowEnvelope = 0.0f;
        sparkBurstEnvelope = 0.0f;
        sparkHighPassInput = 0.0f;
        sparkHighPassOutput = 0.0f;
        sparkLowPassOutput = 0.0f;
        sparkCooldownSamplesRemaining = 0;
        sparkTriggerArmed = true;
        meterState.sparkDetectLeft = 0.0f;
        meterState.sparkDetectRight = 0.0f;
        meterState.sparkThresholdLeft = 0.0f;
        meterState.sparkThresholdRight = 0.0f;
        meterState.sparkDuckLeft = 0.0f;
        meterState.sparkDuckRight = 0.0f;
        return 0.0f;
    }

    const auto sparkInput = sideSample * sparkSend;
    const auto magnitude = std::abs(sparkInput);
    sparkFastEnvelope = sparkFastCoeff * sparkFastEnvelope + (1.0f - sparkFastCoeff) * magnitude;
    sparkSlowEnvelope = sparkSlowCoeff * sparkSlowEnvelope + (1.0f - sparkSlowCoeff) * magnitude;

    const auto transientEnergy = juce::jmax(0.0f, sparkFastEnvelope - sparkSlowEnvelope);
    const auto thresholdScale = juce::jmap(thresholdControl, 0.0f, 100.0f, 0.3f, 1.9f);
    const auto thresholdAmount = thresholdControl * 0.01f;
    const auto threshold =
        juce::jmax(sparkTransientFloor * thresholdScale,
                   sparkSlowEnvelope * sparkTransientSensitivity * thresholdScale);
    const auto thresholdDisplay = juce::jmap(thresholdControl, 0.0f, 100.0f, 0.15f, 0.85f);
    const auto detectDisplay = juce::jlimit(
        0.0f, 1.0f, thresholdDisplay * (transientEnergy / juce::jmax(threshold, 1.0e-6f)));
    meterState.sparkDetectLeft = detectDisplay;
    meterState.sparkDetectRight = detectDisplay;
    meterState.sparkThresholdLeft = thresholdDisplay;
    meterState.sparkThresholdRight = thresholdDisplay;

    if (sparkCooldownSamplesRemaining > 0)
        --sparkCooldownSamplesRemaining;

    const auto canTrigger = sparkTriggerArmed && sparkCooldownSamplesRemaining <= 0;

    if (canTrigger && transientEnergy > threshold) {
        sparkBurstEnvelope =
            juce::jlimit(0.35f, 1.0f, transientEnergy / juce::jmax(threshold * 2.5f, 1.0e-6f));
        sparkCooldownSamplesRemaining = sparkCooldownSamples;
        sparkTriggerArmed = false;
    } else {
        sparkBurstEnvelope *= sparkDecayCoeff;
    }

    if (transientEnergy < threshold * 0.5f)
        sparkTriggerArmed = true;

    const auto highPassed =
        sparkHighPassCoeff * (sparkHighPassOutput + sparkInput - sparkHighPassInput);
    sparkHighPassInput = sparkInput;
    sparkHighPassOutput = highPassed;

    sparkLowPassOutput =
        (1.0f - sparkLowPassCoeff) * highPassed + sparkLowPassCoeff * sparkLowPassOutput;

    const auto thresholdSparkScale = juce::jmap(thresholdAmount, 0.0f, 1.0f, 0.32f, 1.0f);
    auto shapedSpark = sparkLowPassOutput * sparkAmount * thresholdSparkScale * sparkBurstLevel *
                       sparkBurstEnvelope;
    shapedSpark = std::tanh(shapedSpark);
    shapedSpark *= sparkGain;
    shapedSpark = std::tanh(shapedSpark);
    shapedSpark = juce::jlimit(-sparkLimiterCeiling, sparkLimiterCeiling, shapedSpark);
    return shapedSpark;
}

float ProcessorCore::applySparkPitch(float sparkSide, float sparkPitchSemitones) noexcept {
    if (sparkPitchBuffer.empty())
        return sparkSide;

    sparkPitchBuffer[static_cast<std::size_t>(sparkPitchWriteIndex)] = sparkSide;

    const auto ratio = std::pow(2.0f, sparkPitchSemitones / 12.0f);
    const auto bufferSize = static_cast<int>(sparkPitchBuffer.size());
    const auto bufferSizeFloat = static_cast<float>(bufferSize);
    const auto writePosition = static_cast<float>(sparkPitchWriteIndex);

    auto lag = writePosition - sparkPitchReadPosition;
    if (lag < 0.0f)
        lag += bufferSizeFloat;

    const auto targetLag =
        juce::jlimit(sparkPitchMinLagSamples, bufferSizeFloat - sparkPitchMinLagSamples,
                     static_cast<float>((sparkPitchLagMs * 0.001) * currentSampleRate));
    if (lag < sparkPitchMinLagSamples || lag > (bufferSizeFloat - sparkPitchMinLagSamples))
        sparkPitchReadPosition = writePosition - targetLag;

    while (sparkPitchReadPosition < 0.0f)
        sparkPitchReadPosition += bufferSizeFloat;
    while (sparkPitchReadPosition >= bufferSizeFloat)
        sparkPitchReadPosition -= bufferSizeFloat;

    const auto readIndexA = static_cast<int>(sparkPitchReadPosition);
    const auto readIndexB = (readIndexA + 1) % bufferSize;
    const auto fraction = sparkPitchReadPosition - static_cast<float>(readIndexA);
    const auto sampleA = sparkPitchBuffer[static_cast<std::size_t>(readIndexA)];
    const auto sampleB = sparkPitchBuffer[static_cast<std::size_t>(readIndexB)];
    const auto pitchedSpark = juce::jmap(fraction, sampleA, sampleB);

    sparkPitchReadPosition += ratio;
    while (sparkPitchReadPosition >= bufferSizeFloat)
        sparkPitchReadPosition -= bufferSizeFloat;

    sparkPitchWriteIndex = (sparkPitchWriteIndex + 1) % bufferSize;
    return pitchedSpark;
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
    sparkFastCoeff = coefficientForTimeMs(sparkFastTimeMs, currentSampleRate);
    sparkSlowCoeff = coefficientForTimeMs(sparkSlowTimeMs, currentSampleRate);
    sparkDecayCoeff = coefficientForTimeMs(sparkLengthMs, currentSampleRate);
    sparkCooldownSamples =
        juce::jmax(1, static_cast<int>(std::round((sparkCooldownMs * 0.001) * currentSampleRate)));
    const auto pitchBufferSamples = juce::jmax(
        128, static_cast<int>(std::round((sparkPitchBufferMs * 0.001) * currentSampleRate)));
    sparkPitchBuffer.assign(static_cast<std::size_t>(pitchBufferSamples), 0.0f);
    sparkPitchWriteIndex = 0;
    sparkPitchReadPosition =
        static_cast<float>(pitchBufferSamples) -
        juce::jmin(static_cast<float>(pitchBufferSamples) * 0.5f,
                   static_cast<float>((sparkPitchLagMs * 0.001) * currentSampleRate));
    const auto dt = 1.0f / static_cast<float>(currentSampleRate);
    const auto hpRc = 1.0f / (juce::MathConstants<float>::twoPi * sparkHighPassFrequency);
    sparkHighPassCoeff = hpRc / (hpRc + dt);
    sparkLowPassCoeff = std::exp(-juce::MathConstants<float>::twoPi * sparkLowPassFrequency * dt);
}

void ProcessorCore::clearMeters() noexcept {
    meterState = {};
    sparkFastEnvelope = 0.0f;
    sparkSlowEnvelope = 0.0f;
    sparkBurstEnvelope = 0.0f;
    sparkHighPassInput = 0.0f;
    sparkHighPassOutput = 0.0f;
    sparkLowPassOutput = 0.0f;
    sparkPitchBuffer.clear();
    sparkPitchWriteIndex = 0;
    sparkPitchReadPosition = 0.0f;
    sparkCooldownSamplesRemaining = 0;
    sparkTriggerArmed = true;
}

}
