#include "LevelMeter.h"

namespace {

const auto accentPrimary = juce::Colour::fromRGB(101, 226, 202);
const auto accentSecondary = juce::Colour::fromRGB(243, 165, 84);
const auto textPrimary = juce::Colour::fromRGB(240, 243, 247);
const auto textSecondary = juce::Colour::fromRGB(186, 194, 206);

} // namespace

namespace axis::ui::components {

LevelMeter::LevelMeter(juce::String title) {
    titleLabel.setText(title, juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, textSecondary);
    addAndMakeVisible(titleLabel);
}

void LevelMeter::setLevels(float newLeftLevel, float newRightLevel) {
    leftLevel = newLeftLevel;
    rightLevel = newRightLevel;
    repaint();
}

void LevelMeter::paint(juce::Graphics &g) {
    drawBar(g, leftMeterBounds, leftLevel);
    drawBar(g, rightMeterBounds, rightLevel);
}

void LevelMeter::resized() {
    auto area = getLocalBounds();
    titleLabel.setBounds(area.removeFromTop(18));

    auto columns = area.reduced(2, 6);
    const auto meterGap = juce::jmax(4, columns.getWidth() / 12);
    const auto meterWidth = (columns.getWidth() - meterGap) / 2;
    leftMeterBounds = columns.removeFromLeft(meterWidth);
    columns.removeFromLeft(meterGap);
    rightMeterBounds = columns;
}

juce::Colour LevelMeter::colourForLevel(float currentLevel) {
    constexpr auto minusInfinityDb = -100.0f;
    const auto levelDb =
        juce::Decibels::gainToDecibels(juce::jmax(currentLevel, 0.00001f), minusInfinityDb);

    if (levelDb > 0.0f)
        return juce::Colours::white;

    if (levelDb >= -3.0f)
        return juce::Colour::fromRGB(255, 96, 96);

    if (levelDb >= -12.0f)
        return accentSecondary.brighter(0.1f);

    return accentPrimary;
}

void LevelMeter::drawBar(juce::Graphics &g, juce::Rectangle<int> meterBounds, float level) const {
    g.setColour(juce::Colours::black.withAlpha(0.25f));
    g.fillRoundedRectangle(meterBounds.toFloat(), 6.0f);

    auto fillBounds = meterBounds.reduced(3);
    const auto fillHeight =
        juce::roundToInt(fillBounds.getHeight() * juce::jlimit(0.0f, 1.0f, level));
    if (fillHeight > 0) {
        auto activeBounds = fillBounds.withTop(fillBounds.getBottom() - fillHeight);
        g.setColour(colourForLevel(level));
        g.fillRoundedRectangle(activeBounds.toFloat(), 4.0f);
    }

    g.setColour(textPrimary.withAlpha(0.15f));
    g.drawRoundedRectangle(meterBounds.toFloat(), 6.0f, 1.0f);
}

} // namespace axis::ui::components
