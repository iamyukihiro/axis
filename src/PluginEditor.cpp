#include "PluginEditor.h"
#include <BinaryData.h>

namespace
{
const auto brandBlue = juce::Colour::fromString("ff559ec7");
const auto brandBlueDark = juce::Colour::fromString("ff3d789a");
const auto panelOuter = juce::Colour::fromRGB(7, 11, 16);
const auto panelTop = juce::Colour::fromRGB(24, 38, 49);
const auto panelBottom = juce::Colour::fromRGB(11, 18, 24);
const auto frameColour = juce::Colours::white.withAlpha(0.1f);
const auto textPrimary = juce::Colour::fromRGB(240, 243, 247);
const auto textSecondary = juce::Colour::fromRGB(186, 194, 206);
const auto accentSoft = brandBlueDark;
const auto accentStrong = brandBlue.brighter(0.15f);
const auto buttonOff = juce::Colour::fromRGB(28, 41, 51);
const auto buttonOn = brandBlueDark;

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
        const auto background = active ? buttonOn : buttonOff;

        g.setColour(background.brighter(shouldDrawButtonAsHighlighted ? 0.08f : 0.0f));
        g.fillRoundedRectangle(bounds, 8.0f);

        g.setColour(textPrimary.withAlpha(shouldDrawButtonAsDown ? 0.9f : 0.75f));
        g.drawRoundedRectangle(bounds, 8.0f, 1.0f);
    }

    juce::Font getTextButtonFont(juce::TextButton&, int) override
    {
        return juce::Font(juce::FontOptions(16.0f));
    }
};
}

AxisCenterAudioProcessorEditor::AxisCenterAudioProcessorEditor(AxisCenterAudioProcessor& p)
    : AudioProcessorEditor(&p), axisProcessor(p)
{
    setSize(520, 280);

    logoDrawable = juce::Drawable::createFromImageData(BinaryData::logo_svg, BinaryData::logo_svgSize);

    titleLabel.setText("Axis Center", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::Font(juce::FontOptions(24.0f)));
    titleLabel.setColour(juce::Label::textColourId, textPrimary);
    addAndMakeVisible(titleLabel);

    configureSlider(centerGainSlider, "Center Gain");
    configureSlider(sideGainSlider, "Side Gain");
    configureSlider(widthSlider, "Side Width");
    configureSlider(lowMonoSlider, "Low-End Mono");
    configureSlider(outputGainSlider, "Output Gain");
    lowMonoSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);

    configureToggle(softClipButton, "Soft Clip");
    configureToggle(bypassButton, "Bypass");
    resetButton.setButtonText("Reset");
    resetButton.onClick = [this] { axisProcessor.resetParametersToDefault(); };
    resetButton.setColour(juce::TextButton::buttonColourId, buttonOff);
    resetButton.setColour(juce::TextButton::textColourOffId, textPrimary);
    resetButton.setColour(juce::TextButton::buttonOnColourId, buttonOn);
    resetButton.setTriggeredOnMouseDown(false);
    addAndMakeVisible(resetButton);

    static ButtonLookAndFeel buttonLookAndFeel;
    softClipButton.setLookAndFeel(&buttonLookAndFeel);
    bypassButton.setLookAndFeel(&buttonLookAndFeel);
    resetButton.setLookAndFeel(&buttonLookAndFeel);

    configureLabel(centerGainLabel, "Center Gain");
    configureLabel(sideGainLabel, "Side Gain");
    configureLabel(widthLabel, "Side Width");
    configureLabel(lowMonoLabel, "Low-End Mono");
    configureLabel(outputGainLabel, "Output Gain");
    configureLabel(meterLabel, "Output");

    centerGainAttachment = std::make_unique<SliderAttachment>(axisProcessor.apvts, "centerGain", centerGainSlider);
    sideGainAttachment = std::make_unique<SliderAttachment>(axisProcessor.apvts, "sideGain", sideGainSlider);
    widthAttachment = std::make_unique<SliderAttachment>(axisProcessor.apvts, "width", widthSlider);
    lowMonoAttachment = std::make_unique<SliderAttachment>(axisProcessor.apvts, "lowMonoFrequency", lowMonoSlider);
    outputGainAttachment = std::make_unique<SliderAttachment>(axisProcessor.apvts, "outputGain", outputGainSlider);
    softClipAttachment = std::make_unique<ButtonAttachment>(axisProcessor.apvts, "softClip", softClipButton);
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
                    return brandBlue.brighter(0.1f);

                return brandBlueDark;
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
    auto header = area.removeFromTop(42);
    titleLabel.setBounds(header.withTrimmedLeft(92));
    area.removeFromTop(8);

    auto buttons = area.removeFromBottom(38);
    area.removeFromBottom(10);
    auto meterArea = area.removeFromRight(52);
    area.removeFromRight(8);
    auto sliders = area;
    const auto sliderWidth = sliders.getWidth() / 5;

    auto layoutSlider = [] (juce::Rectangle<int> bounds, juce::Slider& slider, juce::Label& label)
    {
        auto cell = bounds.reduced(6);
        label.setBounds(cell.removeFromTop(18));
        slider.setBounds(cell);
    };

    layoutSlider(sliders.removeFromLeft(sliderWidth), centerGainSlider, centerGainLabel);
    layoutSlider(sliders.removeFromLeft(sliderWidth), sideGainSlider, sideGainLabel);
    layoutSlider(sliders.removeFromLeft(sliderWidth), widthSlider, widthLabel);
    layoutSlider(sliders.removeFromLeft(sliderWidth), lowMonoSlider, lowMonoLabel);
    layoutSlider(sliders.removeFromLeft(sliderWidth), outputGainSlider, outputGainLabel);

    meterLabel.setBounds(meterArea.removeFromTop(18));
    auto meterColumns = meterArea.reduced(4, 6);
    leftMeterBounds = meterColumns.removeFromLeft((meterColumns.getWidth() - 6) / 2);
    meterColumns.removeFromLeft(6);
    rightMeterBounds = meterColumns;

    softClipButton.setBounds(buttons.removeFromLeft(160));
    buttons.removeFromLeft(12);
    bypassButton.setBounds(buttons.removeFromLeft(140));
    resetButton.setBounds(buttons.removeFromRight(110));
}

void AxisCenterAudioProcessorEditor::configureSlider(juce::Slider& slider, const juce::String& name)
{
    slider.setName(name);
    slider.setSliderStyle(juce::Slider::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 20);
    slider.setColour(juce::Slider::trackColourId, accentSoft.withAlpha(0.28f));
    slider.setColour(juce::Slider::thumbColourId, accentStrong);
    slider.setColour(juce::Slider::backgroundColourId, juce::Colours::black.withAlpha(0.22f));
    slider.setColour(juce::Slider::textBoxTextColourId, textPrimary);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour::fromRGB(18, 23, 30).withAlpha(0.92f));
    slider.setTextValueSuffix(name == "Side Width" ? " %" : "");
    addAndMakeVisible(slider);
}

void AxisCenterAudioProcessorEditor::configureToggle(juce::Button& button, const juce::String& name)
{
    button.setButtonText(name);
    button.setClickingTogglesState(true);
    button.setColour(juce::TextButton::buttonColourId, buttonOff);
    button.setColour(juce::TextButton::buttonOnColourId, buttonOn);
    button.setColour(juce::TextButton::textColourOffId, textPrimary);
    button.setColour(juce::TextButton::textColourOnId, accentStrong);
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
