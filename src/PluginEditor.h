#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"
#include "ui/components/LevelMeter.h"

class AxisCenterAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                             private juce::Timer {
  public:
    explicit AxisCenterAudioProcessorEditor(AxisCenterAudioProcessor &);
    ~AxisCenterAudioProcessorEditor() override = default;

    void paint(juce::Graphics &) override;
    void resized() override;

  private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    static juce::Colour sliderThumbFallback();
    static juce::Colour sliderTrackFallback();
    static juce::Colour labelTextFallback();
    void configureSlider(juce::Slider &slider, const juce::String &name,
                         juce::Colour thumbColour = sliderThumbFallback(),
                         juce::Colour trackColour = sliderTrackFallback());
    void configureToggle(juce::Button &button, const juce::String &name);
    void configureLabel(juce::Label &label, const juce::String &text,
                        juce::Colour textColour = labelTextFallback());
    void timerCallback() override;

    AxisCenterAudioProcessor &axisProcessor;
    std::unique_ptr<juce::Drawable> logoDrawable;

    juce::Label titleLabel;
    juce::Label versionLabel;
    juce::Label inputLabel;
    juce::Label centerLabel;
    juce::Label sideGainLabel;
    juce::Label densityLabel;
    juce::Label sideSparkLabel;
    juce::Label sparkSendLabel;
    juce::Label sparkGainLabel;
    juce::Label sparkDuckLabel;
    juce::Label sparkThresholdLabel;
    juce::Label widthLabel;
    juce::Label sparkPitchLabel;
    juce::Label outputLabel;
    axis::ui::components::LevelMeter inputMeter{"In"};
    axis::ui::components::LevelMeter detectMeter{"Trigger"};
    axis::ui::components::LevelMeter sparkMeter{"Spark"};
    axis::ui::components::LevelMeter outputMeter{"Out"};
    float displayedInputLeftPeak = 0.0f;
    float displayedInputRightPeak = 0.0f;
    float displayedDetectLeft = 0.0f;
    float displayedDetectRight = 0.0f;
    float displayedSparkLeftPeak = 0.0f;
    float displayedSparkRightPeak = 0.0f;
    float displayedOutputLeftPeak = 0.0f;
    float displayedOutputRightPeak = 0.0f;

    juce::Slider inputSlider;
    juce::Slider centerGainSlider;
    juce::Slider sideGainSlider;
    juce::Slider densitySlider;
    juce::Slider sideSparkSlider;
    juce::Slider sparkSendSlider;
    juce::Slider sparkGainSlider;
    juce::Slider sparkDuckSlider;
    juce::Slider sparkThresholdSlider;
    juce::Slider widthSlider;
    juce::Slider sparkPitchSlider;
    juce::Slider outputSlider;
    juce::TextButton autoGainButton;
    juce::TextButton softClipButton;
    juce::TextButton sparkSoloButton;
    juce::TextButton bypassButton;
    juce::TextButton resetButton;

    std::unique_ptr<SliderAttachment> inputAttachment;
    std::unique_ptr<SliderAttachment> centerGainAttachment;
    std::unique_ptr<SliderAttachment> sideGainAttachment;
    std::unique_ptr<SliderAttachment> densityAttachment;
    std::unique_ptr<SliderAttachment> sideSparkAttachment;
    std::unique_ptr<SliderAttachment> sparkSendAttachment;
    std::unique_ptr<SliderAttachment> sparkGainAttachment;
    std::unique_ptr<SliderAttachment> sparkDuckAttachment;
    std::unique_ptr<SliderAttachment> sparkThresholdAttachment;
    std::unique_ptr<SliderAttachment> widthAttachment;
    std::unique_ptr<SliderAttachment> sparkPitchAttachment;
    std::unique_ptr<SliderAttachment> outputAttachment;
    std::unique_ptr<ButtonAttachment> autoGainAttachment;
    std::unique_ptr<ButtonAttachment> softClipAttachment;
    std::unique_ptr<ButtonAttachment> sparkSoloAttachment;
    std::unique_ptr<ButtonAttachment> bypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AxisCenterAudioProcessorEditor)
};
