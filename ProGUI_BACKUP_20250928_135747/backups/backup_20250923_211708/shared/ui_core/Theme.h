#pragma once
#include "../../JUCE/modules/juce_core/juce_core.h"
#include "../../JUCE/modules/juce_graphics/juce_graphics.h"

namespace ui {
struct Theme {
    // Colors
    juce::Colour bg { 0xFF101318 };
    juce::Colour panel { 0xFF181C24 };
    juce::Colour card { 0xFF1F2632 };
    juce::Colour accent { 0xFF9B78FF };
    juce::Colour accent2 { 0xFF64C8FF };
    juce::Colour text { 0xFFECEFF4 };
    juce::Colour textDim { 0xFFA8B0BD };

    // Metrics
    float r_sm { 8.0f }, r_md { 12.0f }, r_lg { 16.0f };
    float elev_sm { 6.0f }, elev_md { 10.0f }, elev_lg { 14.0f };

    // Typography
    juce::Font title { 18.0f, juce::Font::bold };
    juce::Font body { 14.0f };
};

inline const Theme& theme() { static Theme t; return t; }
} // namespace ui
