#include "PluginEditor.h"
#include <BinaryData.h>

namespace
{
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

class ButtonLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    void drawButtonBackground(juce::Graphics& g,
                              juce::Button& button,
                              const juce::Colour&,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override
    {
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

AxisCenterAudioProcessorEditor::AxisCenterAudioProcessorEditor(AxisCenterAudioProcessor& p)
    : AudioProcessorEditor(&p), axisProcessor(p)
{
    setSize(520, 300);
    logoDrawable = juce::Drawable::createFromImageData(BinaryData::logo_svg, BinaryData::logo_svgSize);

    titleLabel.setText("Axis", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::Font(juce::FontOptions(28.0f)));
    titleLabel.setColour(juce::Label::textColourId, textPrimary);
    addAndMakeVisible(titleLabel);

    versionLabel.setText("Version " + juce::String(ProjectInfo::versionString), juce::dontSendNotification);
    versionLabel.setJustificationType(juce::Justification::centredRight);
    versionLabel.setFont(juce::Font(juce::FontOptions(13.0f)));
    versionLabel.setColour(juce::Label::textColourId, textSecondary);
    addAndMakeVisible(versionLabel);

    configureSlider(inputSlider, "Input");
    configureSlider(centerGainSlider, "Center");
    configureSlider(sideGainSlider, "Side Gain");
    configureSlider(densitySlider, "Side Density");
    configureSlider(widthSlider, "Width");
    configureSlider(outputSlider, "Output");
    configureToggle(autoGainButton, "Auto Gain");
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
    bypassButton.setLookAndFeel(&buttonLookAndFeel);
    resetButton.setLookAndFeel(&buttonLookAndFeel);

    configureLabel(inputLabel, "Input");
    configureLabel(centerLabel, "Center");
    configureLabel(sideGainLabel, "Side Gain");
    configureLabel(densityLabel, "Side Density");
    configureLabel(widthLabel, "Width");
    configureLabel(outputLabel, "Output");
    configureLabel(meterLabel, "Output");

    inputAttachment = std::make_unique<SliderAttachment>(axisProcessor.apvts, "input", inputSlider);
    centerGainAttachment = std::make_unique<SliderAttachment>(axisProcessor.apvts, "center", centerGainSlider);
    sideGainAttachment = std::make_unique<SliderAttachment>(axisProcessor.apvts, "sideGain", sideGainSlider);
    densityAttachment = std::make_unique<SliderAttachment>(axisProcessor.apvts, "density", densitySlider);
    widthAttachment = std::make_unique<SliderAttachment>(axisProcessor.apvts, "width", widthSlider);
    outputAttachment = std::make_unique<SliderAttachment>(axisProcessor.apvts, "output", outputSlider);
    autoGainAttachment = std::make_unique<ButtonAttachment>(axisProcessor.apvts, "autoGain", autoGainButton);
    bypassAttachment = std::make_unique<ButtonAttachment>(axisProcessor.apvts, "bypass", bypassButton);

    startTimerHz(30);
}

void AxisCenterAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(panelOuter);

    auto bounds = getLocalBounds().toFloat();
    juce::ColourGradient gradient(panelTop, bounds.getTopLeft(),
                                  panelBottom, bounds.getBottomRight(), false);
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds.reduced(10.0f), 14.0f);

    g.setColour(frameColour);
    g.drawRoundedRectangle(bounds.reduced(10.0f), 14.0f, 1.0f);

    if (logoDrawable != nullptr)
    {
        auto logoBounds = juce::Rectangle<float>(22.0f, 18.0f, 92.0f, 48.0f);
        logoDrawable->drawWithin(g, logoBounds, juce::RectanglePlacement::centred, 0.95f);
    }

    auto drawMeter = [&g] (juce::Rectangle<int> meterBounds, float level)
    {
        g.setColour(juce::Colours::black.withAlpha(0.25f));
        g.fillRoundedRectangle(meterBounds.toFloat(), 6.0f);

        auto fillBounds = meterBounds.reduced(3);
        const auto fillHeight = juce::roundToInt(fillBounds.getHeight() * juce::jlimit(0.0f, 1.0f, level));
        if (fillHeight > 0)
        {
            auto activeBounds = fillBounds.withTop(fillBounds.getBottom() - fillHeight);
            const auto colourForLevel = [] (float currentLevel)
            {
                constexpr auto minusInfinityDb = -100.0f;
                const auto levelDb = juce::Decibels::gainToDecibels(juce::jmax(currentLevel, 0.00001f), minusInfinityDb);

                if (levelDb >= -1.0f)
                    return juce::Colour::fromRGB(255, 96, 96);

                if (levelDb >= -12.0f)
                    return accentSecondary.brighter(0.1f);

                return accentPrimary;
            };

            g.setColour(colourForLevel(level));
            g.fillRoundedRectangle(activeBounds.toFloat(), 4.0f);
        }

        g.setColour(textPrimary.withAlpha(0.15f));
        g.drawRoundedRectangle(meterBounds.toFloat(), 6.0f, 1.0f);
    };

    drawMeter(leftMeterBounds, displayedLeftPeak);
    drawMeter(rightMeterBounds, displayedRightPeak);
}

void AxisCenterAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(18);
    auto header = area.removeFromTop(62);
    versionLabel.setBounds(header.removeFromRight(120));
    titleLabel.setBounds(header);
    area.removeFromTop(12);

    auto buttons = area.removeFromBottom(38);
    area.removeFromBottom(10);
    auto meterArea = area.removeFromRight(52);
    area.removeFromRight(8);
    auto sliders = area;
    const auto sliderWidth = sliders.getWidth() / 6;

    auto layoutSlider = [] (juce::Rectangle<int> bounds, juce::Slider& slider, juce::Label& label)
    {
        auto cell = bounds.reduced(6);
        label.setBounds(cell.removeFromTop(18));
        slider.setBounds(cell);
    };

    layoutSlider(sliders.removeFromLeft(sliderWidth), inputSlider, inputLabel);
    layoutSlider(sliders.removeFromLeft(sliderWidth), centerGainSlider, centerLabel);
    layoutSlider(sliders.removeFromLeft(sliderWidth), sideGainSlider, sideGainLabel);
    layoutSlider(sliders.removeFromLeft(sliderWidth), densitySlider, densityLabel);
    layoutSlider(sliders.removeFromLeft(sliderWidth), widthSlider, widthLabel);
    layoutSlider(sliders.removeFromLeft(sliderWidth), outputSlider, outputLabel);

    meterLabel.setBounds(meterArea.removeFromTop(18));
    auto meterColumns = meterArea.reduced(4, 6);
    leftMeterBounds = meterColumns.removeFromLeft((meterColumns.getWidth() - 6) / 2);
    meterColumns.removeFromLeft(6);
    rightMeterBounds = meterColumns;

    autoGainButton.setBounds(buttons.removeFromLeft(140));
    buttons.removeFromLeft(12);
    bypassButton.setBounds(buttons.removeFromLeft(140));
    buttons.removeFromLeft(12);
    resetButton.setBounds(buttons.removeFromLeft(110));
}

void AxisCenterAudioProcessorEditor::configureSlider(juce::Slider& slider, const juce::String& name)
{
    slider.setName(name);
    slider.setSliderStyle(juce::Slider::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 20);
    slider.setColour(juce::Slider::trackColourId, sliderTrack);
    slider.setColour(juce::Slider::thumbColourId, sliderThumb);
    slider.setColour(juce::Slider::backgroundColourId, juce::Colours::black.withAlpha(0.22f));
    slider.setColour(juce::Slider::textBoxTextColourId, textPrimary);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, sliderTextBox);
    slider.setTextValueSuffix(name == "Center" || name == "Side Gain" || name == "Output" ? " dB" : " %");
    addAndMakeVisible(slider);
}

void AxisCenterAudioProcessorEditor::configureToggle(juce::Button& button, const juce::String& name)
{
    button.setButtonText(name);
    button.setClickingTogglesState(true);
    button.setColour(juce::TextButton::buttonColourId, resetButtonColour);
    button.setColour(juce::TextButton::buttonOnColourId, accentPrimary.darker(0.6f));
    button.setColour(juce::TextButton::textColourOffId, textPrimary);
    button.setColour(juce::TextButton::textColourOnId, textPrimary);
    addAndMakeVisible(button);
}

void AxisCenterAudioProcessorEditor::configureLabel(juce::Label& label, const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, textSecondary);
    addAndMakeVisible(label);
}

void AxisCenterAudioProcessorEditor::timerCallback()
{
    const auto decay = 0.82f;
    displayedLeftPeak = juce::jmax(axisProcessor.getOutputPeakLeft(), displayedLeftPeak * decay);
    displayedRightPeak = juce::jmax(axisProcessor.getOutputPeakRight(), displayedRightPeak * decay);
    repaint();
}
