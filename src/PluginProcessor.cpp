#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

namespace
{
constexpr auto editorWidth = 520;
constexpr auto editorHeight = 300;
constexpr auto densityRatio = 4.0f;
constexpr auto densityKneeWidthDb = 6.0f;
constexpr auto autoTrimFloor = 0.5f;

juce::String formatDecibelValue(float value)
{
    return juce::String(value, 1) + " dB";
}
}

AxisCenterAudioProcessor::AxisCenterAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    centerParam = apvts.getRawParameterValue(parameterIdToString(ParameterId::center));
    sideGainParam = apvts.getRawParameterValue(parameterIdToString(ParameterId::sideGain));
    densityParam = apvts.getRawParameterValue(parameterIdToString(ParameterId::density));
    widthParam = apvts.getRawParameterValue(parameterIdToString(ParameterId::width));
    outputParam = apvts.getRawParameterValue(parameterIdToString(ParameterId::output));
    bypassParam = apvts.getRawParameterValue(parameterIdToString(ParameterId::bypass));
}

void AxisCenterAudioProcessor::prepareToPlay(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    detectorState = 0.0f;
    compressorGain = 1.0f;
    autoTrimGain = 1.0f;
    outputPeakLeft.store(0.0f);
    outputPeakRight.store(0.0f);
    updateTiming();
}

void AxisCenterAudioProcessor::releaseResources()
{
}

bool AxisCenterAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void AxisCenterAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto channel = totalNumInputChannels; channel < totalNumOutputChannels; ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    if (buffer.getNumChannels() < 2)
    {
        outputPeakLeft.store(0.0f);
        outputPeakRight.store(0.0f);
        return;
    }

    const auto centerGain = dbToLinear(centerParam->load());
    const auto sideGain = dbToLinear(sideGainParam->load());
    const auto densityAmount = juce::jlimit(0.0f, 1.0f, densityParam->load() * 0.01f);
    const auto width = widthParam->load() * 0.01f;
    const auto outputGain = dbToLinear(outputParam->load());
    const auto bypassEnabled = bypassParam->load() >= 0.5f;

    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);
    auto blockPeakLeft = 0.0f;
    auto blockPeakRight = 0.0f;

    for (auto sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const auto inputLeft = left[sample];
        const auto inputRight = right[sample];

        if (bypassEnabled)
        {
            blockPeakLeft = juce::jmax(blockPeakLeft, std::abs(inputLeft));
            blockPeakRight = juce::jmax(blockPeakRight, std::abs(inputRight));
            continue;
        }

        auto mid = 0.5f * (inputLeft + inputRight);
        auto side = 0.5f * (inputLeft - inputRight);

        mid *= centerGain;
        side *= sideGain;
        side = applyDensity(side, densityAmount);
        side *= width;

        auto outLeft = mid + side;
        auto outRight = mid - side;

        const auto trim = applyAutoTrim(inputLeft, inputRight, outLeft, outRight);
        outLeft *= trim * outputGain;
        outRight *= trim * outputGain;

        outLeft = std::tanh(outLeft);
        outRight = std::tanh(outRight);

        left[sample] = outLeft;
        right[sample] = outRight;
        blockPeakLeft = juce::jmax(blockPeakLeft, std::abs(outLeft));
        blockPeakRight = juce::jmax(blockPeakRight, std::abs(outRight));
    }

    outputPeakLeft.store(blockPeakLeft);
    outputPeakRight.store(blockPeakRight);
}

juce::AudioProcessorEditor* AxisCenterAudioProcessor::createEditor()
{
    auto* editor = new AxisCenterAudioProcessorEditor(*this);
    editor->setSize(editorWidth, editorHeight);
    return editor;
}

bool AxisCenterAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String AxisCenterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AxisCenterAudioProcessor::acceptsMidi() const
{
    return false;
}

bool AxisCenterAudioProcessor::producesMidi() const
{
    return false;
}

bool AxisCenterAudioProcessor::isMidiEffect() const
{
    return false;
}

double AxisCenterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AxisCenterAudioProcessor::getNumPrograms()
{
    return 1;
}

int AxisCenterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AxisCenterAudioProcessor::setCurrentProgram(int)
{
}

const juce::String AxisCenterAudioProcessor::getProgramName(int)
{
    return {};
}

void AxisCenterAudioProcessor::changeProgramName(int, const juce::String&)
{
}

void AxisCenterAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void AxisCenterAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xmlState = getXmlFromBinary(data, sizeInBytes))
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

void AxisCenterAudioProcessor::resetParametersToDefault()
{
    for (auto* parameter : getParameters())
    {
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->getDefaultValue());
        parameter->endChangeGesture();
    }
}

float AxisCenterAudioProcessor::getOutputPeakLeft() const noexcept
{
    return outputPeakLeft.load();
}

float AxisCenterAudioProcessor::getOutputPeakRight() const noexcept
{
    return outputPeakRight.load();
}

juce::AudioProcessorValueTreeState::ParameterLayout AxisCenterAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        parameterIdToString(ParameterId::center),
        "Center",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.01f),
        0.0f,
        juce::AudioParameterFloatAttributes()
            .withLabel("dB")
            .withStringFromValueFunction([](float value, int) { return formatDecibelValue(value); })));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        parameterIdToString(ParameterId::sideGain),
        "Side Gain",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.01f),
        0.0f,
        juce::AudioParameterFloatAttributes()
            .withLabel("dB")
            .withStringFromValueFunction([](float value, int) { return formatDecibelValue(value); })));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        parameterIdToString(ParameterId::density),
        "Side Density",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f),
        0.0f,
        juce::AudioParameterFloatAttributes()
            .withLabel("%")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        parameterIdToString(ParameterId::width),
        "Width",
        juce::NormalisableRange<float>(0.0f, 200.0f, 0.01f),
        100.0f,
        juce::AudioParameterFloatAttributes()
            .withLabel("%")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        parameterIdToString(ParameterId::output),
        "Output",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.01f),
        0.0f,
        juce::AudioParameterFloatAttributes()
            .withLabel("dB")
            .withStringFromValueFunction([](float value, int) { return formatDecibelValue(value); })));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        parameterIdToString(ParameterId::bypass),
        "Bypass",
        false));

    return { params.begin(), params.end() };
}

const juce::String AxisCenterAudioProcessor::parameterIdToString(ParameterId id)
{
    switch (id)
    {
        case ParameterId::center: return "center";
        case ParameterId::sideGain: return "sideGain";
        case ParameterId::density: return "density";
        case ParameterId::width: return "width";
        case ParameterId::output: return "output";
        case ParameterId::bypass: return "bypass";
    }

    jassertfalse;
    return {};
}

float AxisCenterAudioProcessor::dbToLinear(float dbValue) noexcept
{
    return juce::Decibels::decibelsToGain(dbValue, -24.0f);
}

float AxisCenterAudioProcessor::gainToDb(float gain) noexcept
{
    return juce::Decibels::gainToDecibels(juce::jmax(gain, 0.00001f), -100.0f);
}

float AxisCenterAudioProcessor::coefficientForTimeMs(float timeMs, double sampleRate) noexcept
{
    return std::exp(-1.0f / (0.001f * timeMs * static_cast<float>(sampleRate)));
}

float AxisCenterAudioProcessor::applyDensity(float sideSample, float densityAmount) noexcept
{
    if (densityAmount <= 0.0f)
        return sideSample;

    const auto sidePower = sideSample * sideSample;
    const auto detectorCoeff = sidePower > detectorState ? detectorAttackCoeff : detectorReleaseCoeff;
    detectorState = detectorCoeff * detectorState + (1.0f - detectorCoeff) * sidePower;

    const auto rmsDb = gainToDb(std::sqrt(detectorState + 1.0e-9f));
    const auto thresholdDb = juce::jlimit(-60.0f, 6.0f, rmsDb - 6.0f);
    const auto inputDb = gainToDb(std::abs(sideSample));
    const auto kneeStartDb = thresholdDb - densityKneeWidthDb * 0.5f;
    const auto kneeEndDb = thresholdDb + densityKneeWidthDb * 0.5f;

    auto gainReductionDb = 0.0f;

    if (inputDb > kneeEndDb)
    {
        gainReductionDb = (1.0f - (1.0f / densityRatio)) * (inputDb - thresholdDb);
    }
    else if (inputDb > kneeStartDb)
    {
        const auto x = inputDb - kneeStartDb;
        gainReductionDb = (1.0f - (1.0f / densityRatio)) * (x * x) / (2.0f * densityKneeWidthDb);
    }

    const auto targetGain = dbToLinear(-gainReductionDb);
    const auto gainCoeff = targetGain < compressorGain ? gainAttackCoeff : gainReleaseCoeff;
    compressorGain = gainCoeff * compressorGain + (1.0f - gainCoeff) * targetGain;

    const auto compressedSide = sideSample * compressorGain;
    return juce::jmap(densityAmount, sideSample, compressedSide);
}

float AxisCenterAudioProcessor::applyAutoTrim(float inputLeft, float inputRight, float outputLeft, float outputRight) noexcept
{
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

void AxisCenterAudioProcessor::updateTiming()
{
    detectorAttackCoeff = coefficientForTimeMs(10.0f, currentSampleRate);
    detectorReleaseCoeff = coefficientForTimeMs(100.0f, currentSampleRate);
    gainAttackCoeff = coefficientForTimeMs(10.0f, currentSampleRate);
    gainReleaseCoeff = coefficientForTimeMs(100.0f, currentSampleRate);
    autoTrimAttackCoeff = coefficientForTimeMs(15.0f, currentSampleRate);
    autoTrimReleaseCoeff = coefficientForTimeMs(120.0f, currentSampleRate);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AxisCenterAudioProcessor();
}
