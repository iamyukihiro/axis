#pragma once

#include <JuceHeader.h>

namespace axis::ui::components {

class LevelMeter final : public juce::Component {
  public:
    explicit LevelMeter(juce::String title);

    void setLevels(float leftLevel, float rightLevel);
    void setMarkerLevel(float newMarkerLevel);

    void paint(juce::Graphics &g) override;
    void resized() override;

  private:
    static juce::Colour colourForLevel(float currentLevel);
    void drawBar(juce::Graphics &g, juce::Rectangle<int> meterBounds, float level) const;

    juce::Label titleLabel;
    juce::Rectangle<int> leftMeterBounds;
    juce::Rectangle<int> rightMeterBounds;
    float leftLevel = 0.0f;
    float rightLevel = 0.0f;
    float markerLevel = -1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
};

}
