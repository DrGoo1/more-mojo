#pragma once
#include <JuceHeader.h>

namespace mojo {

/**
 * Component for displaying SVG icons with shadow effects
 * Provides helper methods for creating icons from binary SVG data
 */
class MojoIcon : public juce::Component {
public:
    explicit MojoIcon(std::unique_ptr<juce::Drawable> drawable)
        : svg(std::move(drawable)) {}

    /** Create an icon from binary SVG data */
    static std::unique_ptr<MojoIcon> fromBinarySVG(const void* data, size_t size) {
        auto drawable = juce::Drawable::createFromImageData(data, size);
        return std::make_unique<MojoIcon>(std::unique_ptr<juce::Drawable>(drawable));
    }

    /** Paint the icon with shadow effect */
    void paint(juce::Graphics& g) override {
        if (!svg)
            return;

        auto bounds = getLocalBounds().toFloat();
        
        // Draw shadow first
        juce::DropShadow(juce::Colours::black.withAlpha(0.5f), 8, {0, 3})
            .drawForRectangle(g, bounds.getSmallestIntegerContainer());
        
        // Then draw SVG icon
        svg->drawWithin(g, bounds.reduced(2.f), juce::RectanglePlacement::centred, 1.0f);
    }

private:
    std::unique_ptr<juce::Drawable> svg;
};

}  // namespace mojo
