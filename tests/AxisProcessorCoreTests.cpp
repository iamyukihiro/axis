#include <JuceHeader.h>

#include "dsp/AxisProcessorCore.h"

namespace {

class AxisProcessorCoreTests final : public juce::UnitTest {
  public:
    AxisProcessorCoreTests() : juce::UnitTest("AxisProcessorCore", "axis") {}

    void runTest() override {
        beginTest("bypass 時は波形を変更しない");
        {
            axis::dsp::ProcessorCore core;
            core.prepare(48000.0);

            juce::AudioBuffer<float> buffer(2, 4);
            buffer.setSample(0, 0, 0.25f);
            buffer.setSample(1, 0, -0.25f);
            buffer.setSample(0, 1, -0.5f);
            buffer.setSample(1, 1, 0.5f);
            buffer.setSample(0, 2, 0.1f);
            buffer.setSample(1, 2, 0.1f);
            buffer.setSample(0, 3, -0.3f);
            buffer.setSample(1, 3, -0.2f);

            const auto original = buffer;
            axis::dsp::ParameterSnapshot parameters;
            parameters.bypassEnabled = true;

            core.process(buffer, parameters);

            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
                    expectWithinAbsoluteError(buffer.getSample(channel, sample),
                                              original.getSample(channel, sample), 0.000001f);
        }

        beginTest("spark width 0 では spark 成分が広がらない");
        {
            axis::dsp::ProcessorCore core;
            core.prepare(48000.0);

            juce::AudioBuffer<float> narrowedBuffer(2, 32);
            narrowedBuffer.clear();
            narrowedBuffer.setSample(0, 4, 0.8f);
            narrowedBuffer.setSample(1, 4, -0.8f);

            juce::AudioBuffer<float> dryBuffer(narrowedBuffer);

            axis::dsp::ParameterSnapshot narrowedParameters;
            narrowedParameters.autoGainEnabled = false;
            narrowedParameters.softClipEnabled = false;
            narrowedParameters.sideSparkPercent = 100.0f;
            narrowedParameters.sparkDuckPercent = 0.0f;
            narrowedParameters.widthPercent = 0.0f;

            axis::dsp::ParameterSnapshot dryParameters;
            dryParameters.autoGainEnabled = false;
            dryParameters.softClipEnabled = false;

            core.process(narrowedBuffer, narrowedParameters);
            core.prepare(48000.0);
            core.process(dryBuffer, dryParameters);

            auto accumulatedDifference = 0.0f;
            for (int sample = 0; sample < narrowedBuffer.getNumSamples(); ++sample) {
                accumulatedDifference +=
                    std::abs(narrowedBuffer.getSample(0, sample) - dryBuffer.getSample(0, sample));
            }

            expectLessThan(accumulatedDifference, 0.001f);
        }

        beginTest("meter が入力と出力のピークを返す");
        {
            axis::dsp::ProcessorCore core;
            core.prepare(48000.0);

            juce::AudioBuffer<float> buffer(2, 3);
            buffer.setSample(0, 0, 0.2f);
            buffer.setSample(1, 0, -0.4f);
            buffer.setSample(0, 1, 0.6f);
            buffer.setSample(1, 1, 0.1f);
            buffer.setSample(0, 2, -0.3f);
            buffer.setSample(1, 2, 0.25f);

            axis::dsp::ParameterSnapshot parameters;
            parameters.bypassEnabled = true;

            core.process(buffer, parameters);
            const auto &meterState = core.getMeterState();

            expectWithinAbsoluteError(meterState.inputPeakLeft, 0.6f, 0.000001f);
            expectWithinAbsoluteError(meterState.inputPeakRight, 0.4f, 0.000001f);
            expectWithinAbsoluteError(meterState.sparkDetectLeft, 0.0f, 0.000001f);
            expectWithinAbsoluteError(meterState.sparkThresholdLeft, 0.0f, 0.000001f);
            expectWithinAbsoluteError(meterState.sparkDuckLeft, 0.0f, 0.000001f);
            expectWithinAbsoluteError(meterState.sparkDuckRight, 0.0f, 0.000001f);
            expectWithinAbsoluteError(meterState.sparkPeakLeft, 0.0f, 0.000001f);
            expectWithinAbsoluteError(meterState.sparkPeakRight, 0.0f, 0.000001f);
            expectWithinAbsoluteError(meterState.outputPeakLeft, 0.6f, 0.000001f);
            expectWithinAbsoluteError(meterState.outputPeakRight, 0.4f, 0.000001f);
        }

        beginTest("soft clip を切ると大きい入力をそのまま通す");
        {
            axis::dsp::ProcessorCore core;
            core.prepare(48000.0);

            juce::AudioBuffer<float> buffer(2, 1);
            buffer.setSample(0, 0, 1.5f);
            buffer.setSample(1, 0, -1.5f);

            axis::dsp::ParameterSnapshot parameters;
            parameters.autoGainEnabled = false;
            parameters.softClipEnabled = false;

            core.process(buffer, parameters);

            expectWithinAbsoluteError(buffer.getSample(0, 0), 1.5f, 0.000001f);
            expectWithinAbsoluteError(buffer.getSample(1, 0), -1.5f, 0.000001f);
        }

        beginTest("side spark は side のアタックに短い追加成分を作る");
        {
            axis::dsp::ProcessorCore core;
            core.prepare(48000.0);

            juce::AudioBuffer<float> sparkBuffer(2, 32);
            sparkBuffer.clear();
            sparkBuffer.setSample(0, 4, 0.8f);
            sparkBuffer.setSample(1, 4, -0.8f);

            juce::AudioBuffer<float> dryBuffer(sparkBuffer);

            axis::dsp::ParameterSnapshot sparkParameters;
            sparkParameters.autoGainEnabled = false;
            sparkParameters.softClipEnabled = false;
            sparkParameters.sideSparkPercent = 100.0f;

            axis::dsp::ParameterSnapshot dryParameters;
            dryParameters.autoGainEnabled = false;
            dryParameters.softClipEnabled = false;

            core.process(sparkBuffer, sparkParameters);
            const auto sparkMeterState = core.getMeterState();
            core.prepare(48000.0);
            core.process(dryBuffer, dryParameters);

            auto accumulatedDifference = 0.0f;
            for (int sample = 0; sample < sparkBuffer.getNumSamples(); ++sample) {
                accumulatedDifference +=
                    std::abs(sparkBuffer.getSample(0, sample) - dryBuffer.getSample(0, sample));
            }

            expectGreaterThan(accumulatedDifference, 0.01f);
            expectGreaterThan(sparkMeterState.sparkDetectLeft, 0.001f);
            expectGreaterThan(sparkMeterState.sparkThresholdLeft, 0.1f);
            expectGreaterThan(sparkMeterState.sparkDuckLeft, 0.001f);
            expectGreaterThan(sparkMeterState.sparkDuckRight, 0.001f);
            expectGreaterThan(sparkMeterState.sparkPeakLeft, 0.001f);
            expectGreaterThan(sparkMeterState.sparkPeakRight, 0.001f);
        }

        beginTest("spark solo では spark 成分だけを出力する");
        {
            axis::dsp::ProcessorCore core;
            core.prepare(48000.0);

            juce::AudioBuffer<float> buffer(2, 32);
            buffer.clear();
            buffer.setSample(0, 4, 0.8f);
            buffer.setSample(1, 4, -0.8f);

            axis::dsp::ParameterSnapshot parameters;
            parameters.autoGainEnabled = false;
            parameters.softClipEnabled = false;
            parameters.sideSparkPercent = 100.0f;
            parameters.sparkSoloEnabled = true;

            core.process(buffer, parameters);

            auto accumulatedEnergy = 0.0f;
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
                accumulatedEnergy += std::abs(buffer.getSample(0, sample));

            expectGreaterThan(accumulatedEnergy, 0.01f);
            expectWithinAbsoluteError(buffer.getSample(0, 4), -buffer.getSample(1, 4), 0.000001f);
        }

        beginTest("spark gain を上げると spark 成分が大きくなる");
        {
            axis::dsp::ProcessorCore core;
            core.prepare(48000.0);

            juce::AudioBuffer<float> boostedBuffer(2, 32);
            boostedBuffer.clear();
            boostedBuffer.setSample(0, 4, 0.8f);
            boostedBuffer.setSample(1, 4, -0.8f);

            juce::AudioBuffer<float> defaultBuffer(boostedBuffer);

            axis::dsp::ParameterSnapshot boostedParameters;
            boostedParameters.autoGainEnabled = false;
            boostedParameters.softClipEnabled = false;
            boostedParameters.sideSparkPercent = 100.0f;
            boostedParameters.sparkGainDb = 24.0f;

            axis::dsp::ParameterSnapshot defaultParameters;
            defaultParameters.autoGainEnabled = false;
            defaultParameters.softClipEnabled = false;
            defaultParameters.sideSparkPercent = 100.0f;
            defaultParameters.sparkGainDb = 6.0f;

            core.process(boostedBuffer, boostedParameters);
            const auto boostedMeterState = core.getMeterState();
            core.prepare(48000.0);
            core.process(defaultBuffer, defaultParameters);
            const auto defaultMeterState = core.getMeterState();

            expectGreaterThan(boostedMeterState.sparkPeakLeft, defaultMeterState.sparkPeakLeft);
            expectGreaterThan(boostedMeterState.sparkPeakRight, defaultMeterState.sparkPeakRight);
        }

        beginTest("spark send を下げると検出と spark 成分が控えめになる");
        {
            axis::dsp::ProcessorCore core;
            core.prepare(48000.0);

            juce::AudioBuffer<float> reducedSendBuffer(2, 32);
            reducedSendBuffer.clear();
            reducedSendBuffer.setSample(0, 4, 0.8f);
            reducedSendBuffer.setSample(1, 4, -0.8f);

            juce::AudioBuffer<float> defaultSendBuffer(reducedSendBuffer);

            axis::dsp::ParameterSnapshot reducedSendParameters;
            reducedSendParameters.autoGainEnabled = false;
            reducedSendParameters.softClipEnabled = false;
            reducedSendParameters.sideSparkPercent = 100.0f;
            reducedSendParameters.sparkSendDb = -18.0f;

            axis::dsp::ParameterSnapshot defaultSendParameters;
            defaultSendParameters.autoGainEnabled = false;
            defaultSendParameters.softClipEnabled = false;
            defaultSendParameters.sideSparkPercent = 100.0f;
            defaultSendParameters.sparkSendDb = 0.0f;

            core.process(reducedSendBuffer, reducedSendParameters);
            const auto reducedSendMeterState = core.getMeterState();
            core.prepare(48000.0);
            core.process(defaultSendBuffer, defaultSendParameters);
            const auto defaultSendMeterState = core.getMeterState();

            expectLessThan(reducedSendMeterState.sparkDetectLeft,
                           defaultSendMeterState.sparkDetectLeft);
            expectLessThan(reducedSendMeterState.sparkPeakLeft,
                           defaultSendMeterState.sparkPeakLeft);
        }

        beginTest("spark pitch を変えると spark 波形が変わる");
        {
            axis::dsp::ProcessorCore core;
            core.prepare(48000.0);

            juce::AudioBuffer<float> shiftedBuffer(2, 512);
            shiftedBuffer.clear();
            shiftedBuffer.setSample(0, 4, 0.8f);
            shiftedBuffer.setSample(1, 4, -0.8f);

            juce::AudioBuffer<float> defaultBuffer(shiftedBuffer);

            axis::dsp::ParameterSnapshot shiftedParameters;
            shiftedParameters.autoGainEnabled = false;
            shiftedParameters.softClipEnabled = false;
            shiftedParameters.sideSparkPercent = 100.0f;
            shiftedParameters.sparkPitchSemitones = 12.0f;

            axis::dsp::ParameterSnapshot defaultParameters;
            defaultParameters.autoGainEnabled = false;
            defaultParameters.softClipEnabled = false;
            defaultParameters.sideSparkPercent = 100.0f;

            core.process(shiftedBuffer, shiftedParameters);
            core.prepare(48000.0);
            core.process(defaultBuffer, defaultParameters);

            auto accumulatedDifference = 0.0f;
            for (int sample = 0; sample < shiftedBuffer.getNumSamples(); ++sample) {
                accumulatedDifference +=
                    std::abs(shiftedBuffer.getSample(0, sample) - defaultBuffer.getSample(0, sample));
            }

            expectGreaterThan(accumulatedDifference, 0.01f);
        }

        beginTest("低い spark threshold では spark 量を自動で抑える");
        {
            axis::dsp::ProcessorCore core;
            core.prepare(48000.0);

            juce::AudioBuffer<float> lowThresholdBuffer(2, 32);
            lowThresholdBuffer.clear();
            lowThresholdBuffer.setSample(0, 4, 0.8f);
            lowThresholdBuffer.setSample(1, 4, -0.8f);

            juce::AudioBuffer<float> defaultThresholdBuffer(lowThresholdBuffer);

            axis::dsp::ParameterSnapshot lowThresholdParameters;
            lowThresholdParameters.autoGainEnabled = false;
            lowThresholdParameters.softClipEnabled = false;
            lowThresholdParameters.sideSparkPercent = 50.0f;
            lowThresholdParameters.sparkGainDb = 0.0f;
            lowThresholdParameters.sparkThresholdPercent = 0.0f;

            axis::dsp::ParameterSnapshot defaultThresholdParameters;
            defaultThresholdParameters.autoGainEnabled = false;
            defaultThresholdParameters.softClipEnabled = false;
            defaultThresholdParameters.sideSparkPercent = 50.0f;
            defaultThresholdParameters.sparkGainDb = 0.0f;
            defaultThresholdParameters.sparkThresholdPercent = 50.0f;

            core.process(lowThresholdBuffer, lowThresholdParameters);
            const auto lowThresholdMeterState = core.getMeterState();
            core.prepare(48000.0);
            core.process(defaultThresholdBuffer, defaultThresholdParameters);
            const auto defaultThresholdMeterState = core.getMeterState();

            expectLessThan(lowThresholdMeterState.sparkPeakLeft,
                           defaultThresholdMeterState.sparkPeakLeft);
            expectLessThan(lowThresholdMeterState.sparkPeakRight,
                           defaultThresholdMeterState.sparkPeakRight);
        }

    }
};

static AxisProcessorCoreTests axisProcessorCoreTests;

}
