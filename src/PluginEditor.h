#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class AxisCenterAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                             private juce::Timer
{
public:
    explicit AxisCenterAudioProcessorEditor(AxisCenterAudioProcessor&);
    ~AxisCenterAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    void configureSlider(juce::Slider& slider, const juce::String& name);
    void configureToggle(juce::Button& button, const juce::String& name);
    void configureLabel(juce::Label& label, const juce::String& text);
    void timerCallback() override;

    AxisCenterAudioProcessor& axisProcessor;
    std::unique_ptr<juce::Drawable> logoDrawable;

    juce::Label titleLabel;
    juce::Label versionLabel;
    juce::Label centerLabel;
    juce::Label sideGainLabel;
    juce::Label densityLabel;
    juce::Label widthLabel;
    juce::Label outputLabel;
    juce::Label meterLabel;
    juce::Rectangle<int> leftMeterBounds;
    juce::Rectangle<int> rightMeterBounds;
    float displayedLeftPeak = 0.0f;
    float displayedRightPeak = 0.0f;

    juce::Slider centerGainSlider;
    juce::Slider sideGainSlider;
    juce::Slider densitySlider;
    juce::Slider widthSlider;
    juce::Slider outputSlider;
    juce::TextButton bypassButton;
    juce::TextButton resetButton;

    std::unique_ptr<SliderAttachment> centerGainAttachment;
    std::unique_ptr<SliderAttachment> sideGainAttachment;
    std::unique_ptr<SliderAttachment> densityAttachment;
    std::unique_ptr<SliderAttachment> widthAttachment;
    std::unique_ptr<SliderAttachment> outputAttachment;
    std::unique_ptr<ButtonAttachment> bypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AxisCenterAudioProcessorEditor)
};
