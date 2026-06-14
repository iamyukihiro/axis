#pragma once

#include <JuceHeader.h>

namespace axis::dsp {

struct ParameterSnapshot {
    float inputDb = 0.0f;
    float centerDb = 0.0f;
    float sideGainDb = 0.0f;
    float densityPercent = 0.0f;
    float sideSparkPercent = 0.0f;
    float sparkDuckPercent = 35.0f;
    float widthPercent = 100.0f;
    float outputDb = 0.0f;
    bool autoGainEnabled = true;
    bool softClipEnabled = true;
    bool bypassEnabled = false;
};

struct MeterState {
    float inputPeakLeft = 0.0f;
    float inputPeakRight = 0.0f;
    float sparkPeakLeft = 0.0f;
    float sparkPeakRight = 0.0f;
    float outputPeakLeft = 0.0f;
    float outputPeakRight = 0.0f;
};

class ProcessorCore {
  public:
    void prepare(double sampleRate);
    void process(juce::AudioBuffer<float> &buffer, const ParameterSnapshot &parameters);

    const MeterState &getMeterState() const noexcept;

  private:
    static float dbToLinear(float dbValue) noexcept;
    static float gainToDb(float gain) noexcept;
    static float coefficientForTimeMs(float timeMs, double sampleRate) noexcept;
    float applyDensity(float sideSample, float densityAmount) noexcept;
    float applySideSpark(float sideSample, float sparkAmount) noexcept;
    float applyAutoTrim(float inputLeft, float inputRight, float outputLeft,
                        float outputRight) noexcept;
    void updateTiming();
    void clearMeters() noexcept;

    MeterState meterState;
    double currentSampleRate = 44100.0;
    float detectorState = 0.0f;
    float compressorGain = 1.0f;
    float autoTrimGain = 1.0f;
    float sparkFastEnvelope = 0.0f;
    float sparkSlowEnvelope = 0.0f;
    float sparkBurstEnvelope = 0.0f;
    float sparkHighPassInput = 0.0f;
    float sparkHighPassOutput = 0.0f;
    float sparkLowPassOutput = 0.0f;
    float detectorAttackCoeff = 0.0f;
    float detectorReleaseCoeff = 0.0f;
    float gainAttackCoeff = 0.0f;
    float gainReleaseCoeff = 0.0f;
    float autoTrimAttackCoeff = 0.0f;
    float autoTrimReleaseCoeff = 0.0f;
    float sparkFastCoeff = 0.0f;
    float sparkSlowCoeff = 0.0f;
    float sparkDecayCoeff = 0.0f;
    float sparkHighPassCoeff = 0.0f;
    float sparkLowPassCoeff = 0.0f;
    bool sparkTriggerArmed = true;
};

}
