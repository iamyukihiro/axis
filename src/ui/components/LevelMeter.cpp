#include "LevelMeter.h"

namespace {

const auto accentPrimary = juce::Colour::fromRGB(101, 226, 202);
const auto accentSecondary = juce::Colour::fromRGB(243, 165, 84);
const auto warningColour = juce::Colour::fromRGB(255, 96, 96);
const auto textPrimary = juce::Colour::fromRGB(240, 243, 247);
const auto textSecondary = juce::Colour::fromRGB(186, 194, 206);
constexpr auto yellowThresholdDb = -6.0f;
constexpr auto redThresholdDb = -1.0f;

}

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

void LevelMeter::setMarkerLevel(float newMarkerLevel) {
    markerLevel = juce::jlimit(-1.0f, 1.0f, newMarkerLevel);
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

    if (levelDb >= redThresholdDb)
        return warningColour;

    if (levelDb >= yellowThresholdDb)
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

    if (markerLevel >= 0.0f) {
        const auto markerY = juce::jlimit(
            fillBounds.getY(), fillBounds.getBottom(),
            fillBounds.getBottom() -
                juce::roundToInt(fillBounds.getHeight() * juce::jlimit(0.0f, 1.0f, markerLevel)));
        g.setColour(textSecondary.withAlpha(0.75f));
        g.drawLine(static_cast<float>(fillBounds.getX()), static_cast<float>(markerY),
                   static_cast<float>(fillBounds.getRight()), static_cast<float>(markerY), 1.2f);
    }

    g.setColour(textPrimary.withAlpha(0.15f));
    g.drawRoundedRectangle(meterBounds.toFloat(), 6.0f, 1.0f);
}

}
