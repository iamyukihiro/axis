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

        beginTest("width 0 では出力がモノラルになる");
        {
            axis::dsp::ProcessorCore core;
            core.prepare(48000.0);

            juce::AudioBuffer<float> buffer(2, 1);
            buffer.setSample(0, 0, 0.8f);
            buffer.setSample(1, 0, -0.2f);

            axis::dsp::ParameterSnapshot parameters;
            parameters.widthPercent = 0.0f;
            parameters.autoGainEnabled = false;

            core.process(buffer, parameters);

            expectWithinAbsoluteError(buffer.getSample(0, 0), buffer.getSample(1, 0), 0.000001f);
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
            expectGreaterThan(sparkMeterState.sparkPeakLeft, 0.001f);
            expectGreaterThan(sparkMeterState.sparkPeakRight, 0.001f);
        }
    }
};

static AxisProcessorCoreTests axisProcessorCoreTests;

}
