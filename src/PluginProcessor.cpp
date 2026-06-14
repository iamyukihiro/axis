#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "infrastructure/JuceParameterLayout.h"
#include "infrastructure/JucePluginState.h"

namespace {
constexpr auto editorWidth = 640;
constexpr auto editorHeight = 360;
}

AxisCenterAudioProcessor::AxisCenterAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout()) {
    parameterStore.bind(apvts);
}

void AxisCenterAudioProcessor::prepareToPlay(double sampleRate, int) {
    processorCore.prepare(sampleRate);
    inputPeakLeft.store(0.0f);
    inputPeakRight.store(0.0f);
    outputPeakLeft.store(0.0f);
    outputPeakRight.store(0.0f);
}

void AxisCenterAudioProcessor::releaseResources() {}

bool AxisCenterAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo() &&
           layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void AxisCenterAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &) {
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto channel = totalNumInputChannels; channel < totalNumOutputChannels; ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    if (buffer.getNumChannels() < 2) {
        inputPeakLeft.store(0.0f);
        inputPeakRight.store(0.0f);
        outputPeakLeft.store(0.0f);
        outputPeakRight.store(0.0f);
        return;
    }

    processorCore.process(buffer, parameterStore.snapshot());
    const auto &meterState = processorCore.getMeterState();
    inputPeakLeft.store(meterState.inputPeakLeft);
    inputPeakRight.store(meterState.inputPeakRight);
    outputPeakLeft.store(meterState.outputPeakLeft);
    outputPeakRight.store(meterState.outputPeakRight);
}

juce::AudioProcessorEditor *AxisCenterAudioProcessor::createEditor() {
    auto *editor = new AxisCenterAudioProcessorEditor(*this);
    editor->setSize(editorWidth, editorHeight);
    return editor;
}

bool AxisCenterAudioProcessor::hasEditor() const { return true; }

const juce::String AxisCenterAudioProcessor::getName() const { return JucePlugin_Name; }

bool AxisCenterAudioProcessor::acceptsMidi() const { return false; }

bool AxisCenterAudioProcessor::producesMidi() const { return false; }

bool AxisCenterAudioProcessor::isMidiEffect() const { return false; }

double AxisCenterAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int AxisCenterAudioProcessor::getNumPrograms() { return 1; }

int AxisCenterAudioProcessor::getCurrentProgram() { return 0; }

void AxisCenterAudioProcessor::setCurrentProgram(int) {}

const juce::String AxisCenterAudioProcessor::getProgramName(int) { return {}; }

void AxisCenterAudioProcessor::changeProgramName(int, const juce::String &) {}

void AxisCenterAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {
    axis::infrastructure::writeStateToMemory(apvts, destData);
}

void AxisCenterAudioProcessor::setStateInformation(const void *data, int sizeInBytes) {
    axis::infrastructure::restoreStateFromMemory(apvts, data, sizeInBytes);
}

void AxisCenterAudioProcessor::resetParametersToDefault() { parameterStore.resetToDefault(*this); }

float AxisCenterAudioProcessor::getInputPeakLeft() const noexcept { return inputPeakLeft.load(); }

float AxisCenterAudioProcessor::getInputPeakRight() const noexcept { return inputPeakRight.load(); }

float AxisCenterAudioProcessor::getOutputPeakLeft() const noexcept { return outputPeakLeft.load(); }

float AxisCenterAudioProcessor::getOutputPeakRight() const noexcept {
    return outputPeakRight.load();
}

juce::AudioProcessorValueTreeState::ParameterLayout
AxisCenterAudioProcessor::createParameterLayout() {
    return axis::infrastructure::createParameterLayout();
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() { return new AxisCenterAudioProcessor(); }
