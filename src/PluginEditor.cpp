#include "PluginEditor.h"

#include "domain/AxisParameterModel.h"

#include <BinaryData.h>

namespace {
const auto accentPrimary = juce::Colour::fromRGB(101, 226, 202);
const auto accentSecondary = juce::Colour::fromRGB(243, 165, 84);
const auto panelOuter = juce::Colour::fromRGB(6, 10, 14);
const auto panelTop = juce::Colour::fromRGB(18, 34, 39);
const auto panelBottom = juce::Colour::fromRGB(10, 17, 22);
const auto frameColour = juce::Colours::white.withAlpha(0.1f);
const auto textPrimary = juce::Colour::fromRGB(240, 243, 247);
const auto textSecondary = juce::Colour::fromRGB(186, 194, 206);
const auto sliderTrack = juce::Colour::fromRGB(34, 62, 68);
const auto sliderThumb = accentPrimary;
const auto sliderTextBox = juce::Colour::fromRGB(17, 23, 28).withAlpha(0.96f);
const auto resetButtonColour = juce::Colour::fromRGB(30, 44, 50);

class ButtonLookAndFeel final : public juce::LookAndFeel_V4 {
  public:
    void drawButtonBackground(juce::Graphics &g, juce::Button &button, const juce::Colour &,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override {
        auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
        const auto active = button.getToggleState();
        const auto background = active ? accentPrimary.withAlpha(0.3f) : resetButtonColour;

        g.setColour(background.brighter(shouldDrawButtonAsHighlighted ? 0.08f : 0.0f));
        g.fillRoundedRectangle(bounds, 8.0f);

        g.setColour(textPrimary.withAlpha(shouldDrawButtonAsDown ? 0.9f : 0.75f));
        g.drawRoundedRectangle(bounds, 8.0f, 1.0f);
    }
};
}

AxisCenterAudioProcessorEditor::AxisCenterAudioProcessorEditor(AxisCenterAudioProcessor &p)
    : AudioProcessorEditor(&p), axisProcessor(p) {
    setResizable(true, true);
    setResizeLimits(720, 360, 1040, 640);
    setSize(720, 360);
    logoDrawable =
        juce::Drawable::createFromImageData(BinaryData::logo_svg, BinaryData::logo_svgSize);

    titleLabel.setText("Axis", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setFont(juce::Font(juce::FontOptions(28.0f)));
    titleLabel.setColour(juce::Label::textColourId, textPrimary);
    addAndMakeVisible(titleLabel);

    versionLabel.setText("Version " + juce::String(ProjectInfo::versionString),
                         juce::dontSendNotification);
    versionLabel.setJustificationType(juce::Justification::centredLeft);
    versionLabel.setFont(juce::Font(juce::FontOptions(13.0f)));
    versionLabel.setColour(juce::Label::textColourId, textSecondary);
    addAndMakeVisible(versionLabel);

    configureSlider(inputSlider, "Input");
    configureSlider(centerGainSlider, "Mid");
    configureSlider(sideGainSlider, "Side Gain");
    configureSlider(densitySlider, "Side Density");
    configureSlider(sideSparkSlider, "Side Spark");
    configureSlider(sparkDuckSlider, "Spark Duck");
    configureSlider(widthSlider, "Width");
    configureSlider(outputSlider, "Output");
    configureToggle(autoGainButton, "Auto Gain");
    configureToggle(softClipButton, "Soft Clip");
    configureToggle(bypassButton, "Bypass");
    resetButton.setButtonText("Reset");
    resetButton.onClick = [this] { axisProcessor.resetParametersToDefault(); };
    resetButton.setColour(juce::TextButton::buttonColourId, resetButtonColour);
    resetButton.setColour(juce::TextButton::textColourOffId, textPrimary);
    resetButton.setColour(juce::TextButton::buttonOnColourId, accentPrimary.darker(0.6f));
    resetButton.setTriggeredOnMouseDown(false);
    addAndMakeVisible(resetButton);

    static ButtonLookAndFeel buttonLookAndFeel;
    autoGainButton.setLookAndFeel(&buttonLookAndFeel);
    softClipButton.setLookAndFeel(&buttonLookAndFeel);
    bypassButton.setLookAndFeel(&buttonLookAndFeel);
    resetButton.setLookAndFeel(&buttonLookAndFeel);

    configureLabel(inputLabel, "Input");
    configureLabel(centerLabel, "Mid");
    configureLabel(sideGainLabel, "Side Gain");
    configureLabel(densityLabel, "Side Density");
    configureLabel(sideSparkLabel, "Side Spark");
    configureLabel(sparkDuckLabel, "Spark Duck");
    configureLabel(widthLabel, "Width");
    configureLabel(outputLabel, "Output");
    addAndMakeVisible(inputMeter);
    addAndMakeVisible(sparkMeter);
    addAndMakeVisible(outputMeter);

    using axis::domain::ParameterId;
    using axis::domain::parameterKey;

    inputAttachment = std::make_unique<SliderAttachment>(
        axisProcessor.apvts, juce::String(parameterKey(ParameterId::input).data()), inputSlider);
    centerGainAttachment = std::make_unique<SliderAttachment>(
        axisProcessor.apvts, juce::String(parameterKey(ParameterId::center).data()),
        centerGainSlider);
    sideGainAttachment = std::make_unique<SliderAttachment>(
        axisProcessor.apvts, juce::String(parameterKey(ParameterId::sideGain).data()),
        sideGainSlider);
    densityAttachment = std::make_unique<SliderAttachment>(
        axisProcessor.apvts, juce::String(parameterKey(ParameterId::density).data()),
        densitySlider);
    sideSparkAttachment = std::make_unique<SliderAttachment>(
        axisProcessor.apvts, juce::String(parameterKey(ParameterId::sideSpark).data()),
        sideSparkSlider);
    sparkDuckAttachment = std::make_unique<SliderAttachment>(
        axisProcessor.apvts, juce::String(parameterKey(ParameterId::sparkDuck).data()),
        sparkDuckSlider);
    widthAttachment = std::make_unique<SliderAttachment>(
        axisProcessor.apvts, juce::String(parameterKey(ParameterId::width).data()), widthSlider);
    outputAttachment = std::make_unique<SliderAttachment>(
        axisProcessor.apvts, juce::String(parameterKey(ParameterId::output).data()), outputSlider);
    autoGainAttachment = std::make_unique<ButtonAttachment>(
        axisProcessor.apvts, juce::String(parameterKey(ParameterId::autoGain).data()),
        autoGainButton);
    softClipAttachment = std::make_unique<ButtonAttachment>(
        axisProcessor.apvts, juce::String(parameterKey(ParameterId::softClip).data()),
        softClipButton);
    bypassAttachment = std::make_unique<ButtonAttachment>(
        axisProcessor.apvts, juce::String(parameterKey(ParameterId::bypass).data()), bypassButton);

    startTimerHz(30);
}

void AxisCenterAudioProcessorEditor::paint(juce::Graphics &g) {
    g.fillAll(panelOuter);

    auto bounds = getLocalBounds().toFloat();
    juce::ColourGradient gradient(panelTop, bounds.getTopLeft(), panelBottom,
                                  bounds.getBottomRight(), false);
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds.reduced(10.0f), 14.0f);

    g.setColour(frameColour);
    g.drawRoundedRectangle(bounds.reduced(10.0f), 14.0f, 1.0f);

    if (logoDrawable != nullptr) {
        const auto logoWidth = juce::jlimit(44.0f, 64.0f, getWidth() * 0.1f);
        const auto logoHeight = logoWidth * 0.52f;
        auto logoBounds =
            juce::Rectangle<float>(getWidth() - logoWidth - 24.0f, 20.0f, logoWidth, logoHeight);
        logoDrawable->drawWithin(g, logoBounds, juce::RectanglePlacement::centred, 0.95f);
    }
}

void AxisCenterAudioProcessorEditor::resized() {
    auto area = getLocalBounds().reduced(18);
    const auto compactLayout = getWidth() < 620;
    const auto narrowLayout = getWidth() < 470;
    const auto headerHeight = compactLayout ? 58 : 66;
    const auto logoWidth = juce::jlimit(44, 64, static_cast<int>(getWidth() * 0.1f));
    const auto logoAreaWidth = logoWidth + 28;

    titleLabel.setFont(juce::Font(juce::FontOptions(compactLayout ? 24.0f : 28.0f)));
    versionLabel.setFont(juce::Font(juce::FontOptions(compactLayout ? 11.5f : 13.0f)));

    auto header = area.removeFromTop(headerHeight);
    auto textArea = header;
    textArea.removeFromRight(logoAreaWidth);
    titleLabel.setBounds(textArea.removeFromTop(compactLayout ? 32 : 36));
    versionLabel.setBounds(textArea.removeFromTop(18));
    area.removeFromTop(12);

    auto layoutSlider = [](juce::Rectangle<int> bounds, juce::Slider &slider, juce::Label &label) {
        auto cell = bounds.reduced(6);
        label.setBounds(cell.removeFromTop(18));
        slider.setBounds(cell);
    };

    auto buttons = area.removeFromBottom(compactLayout ? (narrowLayout ? 84 : 80) : 38);
    area.removeFromBottom(10);

    if (!compactLayout) {
        auto meterArea = area.removeFromRight(136);
        area.removeFromRight(8);
        auto sliders = area;
        const auto sliderWidth = sliders.getWidth() / 8;

        layoutSlider(sliders.removeFromLeft(sliderWidth), inputSlider, inputLabel);
        layoutSlider(sliders.removeFromLeft(sliderWidth), centerGainSlider, centerLabel);
        layoutSlider(sliders.removeFromLeft(sliderWidth), sideGainSlider, sideGainLabel);
        layoutSlider(sliders.removeFromLeft(sliderWidth), densitySlider, densityLabel);
        layoutSlider(sliders.removeFromLeft(sliderWidth), sideSparkSlider, sideSparkLabel);
        layoutSlider(sliders.removeFromLeft(sliderWidth), sparkDuckSlider, sparkDuckLabel);
        layoutSlider(sliders.removeFromLeft(sliderWidth), widthSlider, widthLabel);
        layoutSlider(sliders.removeFromLeft(sliderWidth), outputSlider, outputLabel);

        auto meterSections = meterArea.reduced(2, 6);
        const auto sectionGap = 8;
        const auto sectionWidth = (meterSections.getWidth() - (sectionGap * 2)) / 3;
        auto inputSection = meterSections.removeFromLeft(sectionWidth);
        meterSections.removeFromLeft(sectionGap);
        auto sparkSection = meterSections.removeFromLeft(sectionWidth);
        meterSections.removeFromLeft(sectionGap);
        auto outputSection = meterSections;
        inputMeter.setBounds(inputSection);
        sparkMeter.setBounds(sparkSection);
        outputMeter.setBounds(outputSection);
    } else {
        auto meterArea = area.removeFromBottom(narrowLayout ? 94 : 88);
        area.removeFromBottom(6);
        auto sliders = area;

        const int columns = narrowLayout ? 2 : 3;
        constexpr int sliderCount = 8;
        const int rows = (sliderCount + columns - 1) / columns;
        const int cellWidth = sliders.getWidth() / columns;
        const int cellHeight = sliders.getHeight() / rows;

        juce::Slider *sliderComponents[] = {&inputSlider,   &centerGainSlider, &sideGainSlider,
                                            &densitySlider, &sideSparkSlider,  &sparkDuckSlider,
                                            &widthSlider,   &outputSlider};
        juce::Label *sliderLabels[] = {&inputLabel,   &centerLabel,    &sideGainLabel,
                                       &densityLabel, &sideSparkLabel, &sparkDuckLabel,
                                       &widthLabel,   &outputLabel};

        for (int index = 0; index < sliderCount; ++index) {
            const int column = index % columns;
            const int row = index / columns;
            auto cell = juce::Rectangle<int>(
                sliders.getX() + column * cellWidth, sliders.getY() + row * cellHeight,
                column == columns - 1 ? sliders.getRight() - (sliders.getX() + column * cellWidth)
                                      : cellWidth,
                row == rows - 1 ? sliders.getBottom() - (sliders.getY() + row * cellHeight)
                                : cellHeight);
            layoutSlider(cell, *sliderComponents[index], *sliderLabels[index]);
        }

        auto meterSections = meterArea.reduced(8, 4);
        const auto sectionGap = 10;
        const auto sectionWidth = (meterSections.getWidth() - (sectionGap * 2)) / 3;
        auto inputSection = meterSections.removeFromLeft(sectionWidth);
        meterSections.removeFromLeft(sectionGap);
        auto sparkSection = meterSections.removeFromLeft(sectionWidth);
        meterSections.removeFromLeft(sectionGap);
        auto outputSection = meterSections;
        inputMeter.setBounds(inputSection);
        sparkMeter.setBounds(sparkSection);
        outputMeter.setBounds(outputSection);
    }

    if (!compactLayout) {
        autoGainButton.setBounds(buttons.removeFromLeft(120));
        buttons.removeFromLeft(12);
        softClipButton.setBounds(buttons.removeFromLeft(120));
        buttons.removeFromLeft(12);
        bypassButton.setBounds(buttons.removeFromLeft(120));
        buttons.removeFromLeft(12);
        resetButton.setBounds(buttons.removeFromLeft(110));
    } else if (!narrowLayout) {
        const auto buttonWidth = (buttons.getWidth() - 36) / 4;
        autoGainButton.setBounds(buttons.removeFromLeft(buttonWidth));
        buttons.removeFromLeft(12);
        softClipButton.setBounds(buttons.removeFromLeft(buttonWidth));
        buttons.removeFromLeft(12);
        bypassButton.setBounds(buttons.removeFromLeft(buttonWidth));
        buttons.removeFromLeft(12);
        resetButton.setBounds(buttons.removeFromLeft(buttonWidth));
    } else {
        auto topRow = buttons.removeFromTop(36);
        auto bottomRow = buttons.removeFromTop(36);
        const auto halfWidth = (topRow.getWidth() - 10) / 2;

        autoGainButton.setBounds(topRow.removeFromLeft(halfWidth));
        topRow.removeFromLeft(10);
        softClipButton.setBounds(topRow);
        bypassButton.setBounds(bottomRow.removeFromLeft(halfWidth));
        bottomRow.removeFromLeft(10);
        resetButton.setBounds(bottomRow);
    }
}

void AxisCenterAudioProcessorEditor::configureSlider(juce::Slider &slider,
                                                     const juce::String &name) {
    slider.setName(name);
    slider.setSliderStyle(juce::Slider::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 20);
    slider.setColour(juce::Slider::trackColourId, sliderTrack);
    slider.setColour(juce::Slider::thumbColourId, sliderThumb);
    slider.setColour(juce::Slider::backgroundColourId, juce::Colours::black.withAlpha(0.22f));
    slider.setColour(juce::Slider::textBoxTextColourId, textPrimary);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, sliderTextBox);
    addAndMakeVisible(slider);
}

void AxisCenterAudioProcessorEditor::configureToggle(juce::Button &button,
                                                     const juce::String &name) {
    button.setButtonText(name);
    button.setClickingTogglesState(true);
    button.setColour(juce::TextButton::buttonColourId, resetButtonColour);
    button.setColour(juce::TextButton::buttonOnColourId, accentPrimary.darker(0.6f));
    button.setColour(juce::TextButton::textColourOffId, textPrimary);
    button.setColour(juce::TextButton::textColourOnId, textPrimary);
    addAndMakeVisible(button);
}

void AxisCenterAudioProcessorEditor::configureLabel(juce::Label &label, const juce::String &text) {
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, textSecondary);
    addAndMakeVisible(label);
}

void AxisCenterAudioProcessorEditor::timerCallback() {
    const auto decay = 0.82f;
    displayedInputLeftPeak =
        juce::jmax(axisProcessor.getInputPeakLeft(), displayedInputLeftPeak * decay);
    displayedInputRightPeak =
        juce::jmax(axisProcessor.getInputPeakRight(), displayedInputRightPeak * decay);
    displayedSparkLeftPeak =
        juce::jmax(axisProcessor.getSparkPeakLeft(), displayedSparkLeftPeak * decay);
    displayedSparkRightPeak =
        juce::jmax(axisProcessor.getSparkPeakRight(), displayedSparkRightPeak * decay);
    displayedOutputLeftPeak =
        juce::jmax(axisProcessor.getOutputPeakLeft(), displayedOutputLeftPeak * decay);
    displayedOutputRightPeak =
        juce::jmax(axisProcessor.getOutputPeakRight(), displayedOutputRightPeak * decay);
    inputMeter.setLevels(displayedInputLeftPeak, displayedInputRightPeak);
    sparkMeter.setLevels(displayedSparkLeftPeak, displayedSparkRightPeak);
    outputMeter.setLevels(displayedOutputLeftPeak, displayedOutputRightPeak);
}
