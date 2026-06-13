#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

namespace
{
constexpr auto editorWidth = 520;
constexpr auto editorHeight = 280;

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
    centerGainParam = apvts.getRawParameterValue(parameterIdToString(ParameterId::centerGain));
    sideGainParam = apvts.getRawParameterValue(parameterIdToString(ParameterId::sideGain));
    widthParam = apvts.getRawParameterValue(parameterIdToString(ParameterId::width));
    lowMonoFrequencyParam = apvts.getRawParameterValue(parameterIdToString(ParameterId::lowMonoFrequency));
    outputGainParam = apvts.getRawParameterValue(parameterIdToString(ParameterId::outputGain));
    softClipParam = apvts.getRawParameterValue(parameterIdToString(ParameterId::softClip));
    bypassParam = apvts.getRawParameterValue(parameterIdToString(ParameterId::bypass));
}

void AxisCenterAudioProcessor::prepareToPlay(double sampleRate, int)
{
    sideHighPass.prepare(sampleRate);
    outputPeakLeft.store(0.0f);
    outputPeakRight.store(0.0f);
    updateFilters();
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

    if (buffer.getNumChannels() < 2 || *bypassParam >= 0.5f)
    {
        outputPeakLeft.store(0.0f);
        outputPeakRight.store(0.0f);
        return;
    }

    updateFilters();

    const auto centerGain = dbToLinear(centerGainParam->load());
    const auto sideGain = dbToLinear(sideGainParam->load());
    const auto width = widthParam->load() * 0.01f;
    const auto outputGain = dbToLinear(outputGainParam->load());
    const auto softClipEnabled = softClipParam->load() >= 0.5f;
    const auto lowMonoFrequency = lowMonoFrequencyParam->load();
    const auto lowMonoEnabled = lowMonoFrequency >= 20.0f;

    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);
    auto blockPeakLeft = 0.0f;
    auto blockPeakRight = 0.0f;

    for (auto sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        auto mid = 0.5f * (left[sample] + right[sample]);
        auto side = 0.5f * (left[sample] - right[sample]);

        mid *= centerGain;
        side *= sideGain * width;

        if (lowMonoEnabled)
            side = sideHighPass.processSample(side);

        auto outLeft = (mid + side) * outputGain;
        auto outRight = (mid - side) * outputGain;

        if (softClipEnabled)
        {
            outLeft = std::tanh(outLeft);
            outRight = std::tanh(outRight);
        }

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
        parameterIdToString(ParameterId::centerGain),
        "Center Gain",
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
        parameterIdToString(ParameterId::width),
        "Side Width",
        juce::NormalisableRange<float>(0.0f, 200.0f, 0.01f),
        100.0f,
        juce::AudioParameterFloatAttributes()
            .withLabel("%")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        parameterIdToString(ParameterId::lowMonoFrequency),
        "Low-End Mono",
        juce::NormalisableRange<float>(0.0f, 300.0f, 1.0f),
        0.0f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction([](float value, int)
            {
                return value < 20.0f ? "Off" : juce::String(juce::roundToInt(value)) + " Hz";
            })
            .withValueFromStringFunction([](const juce::String& text)
            {
                return text.equalsIgnoreCase("Off") ? 0.0f : text.getFloatValue();
            })));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        parameterIdToString(ParameterId::outputGain),
        "Output Gain",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.01f),
        0.0f,
        juce::AudioParameterFloatAttributes()
            .withLabel("dB")
            .withStringFromValueFunction([](float value, int) { return formatDecibelValue(value); })));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        parameterIdToString(ParameterId::softClip),
        "Soft Clip",
        false));

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
        case ParameterId::centerGain: return "centerGain";
        case ParameterId::sideGain: return "sideGain";
        case ParameterId::width: return "width";
        case ParameterId::lowMonoFrequency: return "lowMonoFrequency";
        case ParameterId::outputGain: return "outputGain";
        case ParameterId::softClip: return "softClip";
        case ParameterId::bypass: return "bypass";
    }

    jassertfalse;
    return {};
}

float AxisCenterAudioProcessor::dbToLinear(float dbValue) noexcept
{
    return juce::Decibels::decibelsToGain(dbValue, -24.0f);
}

void AxisCenterAudioProcessor::updateFilters()
{
    const auto frequency = lowMonoFrequencyParam->load();

    sideHighPass.setCutoff(frequency >= 20.0f ? frequency : 0.0f);
}

void AxisCenterAudioProcessor::OnePoleHighPass::prepare(double newSampleRate)
{
    sampleRate = newSampleRate;
    reset();
    setCutoff(cutoffHz);
}

void AxisCenterAudioProcessor::OnePoleHighPass::setCutoff(float newCutoffHz)
{
    cutoffHz = newCutoffHz;

    if (cutoffHz <= 0.0f || sampleRate <= 0.0)
    {
        alpha = 0.0f;
        return;
    }

    const auto rc = 1.0f / (juce::MathConstants<float>::twoPi * cutoffHz);
    const auto dt = 1.0f / static_cast<float>(sampleRate);
    alpha = rc / (rc + dt);
}

float AxisCenterAudioProcessor::OnePoleHighPass::processSample(float inputSample) noexcept
{
    if (cutoffHz <= 0.0f)
        return inputSample;

    const auto output = alpha * (previousOutput + inputSample - previousInput);
    previousInput = inputSample;
    previousOutput = output;
    return output;
}

void AxisCenterAudioProcessor::OnePoleHighPass::reset() noexcept
{
    previousInput = 0.0f;
    previousOutput = 0.0f;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AxisCenterAudioProcessor();
}
