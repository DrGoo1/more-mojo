#pragma once
#include <JuceHeader.h>

// Simple skin manager stub. Not wired to build by default.
// Extend to load PNGs/ZIPs from Resources/skins/<SkinName>/ and provide image handles
// and parameters (colors, sizes). Falls back to vector draws when assets missing.
class SkinManager {
public:
    struct Skin {
        juce::String name { "Default" };
        // Example assets (extend as needed)
        std::unique_ptr<juce::Image> knobBody;
        std::unique_ptr<juce::Image> knobRing;
        std::unique_ptr<juce::Image> sliderTrack;
        std::unique_ptr<juce::Image> sliderThumb;
        std::unique_ptr<juce::Image> meterOverlay;

        // Theme overrides
        juce::Colour accent { 0xFF9B78FF };
        juce::Colour accent2{ 0xFF5AA8FF };
        juce::Colour panel  { 0xFF1F2632 };
        juce::Colour text   { 0xFFECEFF4 };
    };

    static SkinManager& instance() { static SkinManager mgr; return mgr; }

    // Attempts to load a skin from folder; if not found, keeps Default
    bool loadSkin(const juce::File& folder) {
        current.name = folder.getFileName();
        // TODO: Implement actual loading logic (PNG decode, @2x, etc.)
        // This stub just logs and returns true.
        logToFile("[SkinManager] Requested load: " + folder.getFullPathName());
        return true;
    }

    const Skin& skin() const { return current; }

    static void logToFile(const juce::String& s) {
        juce::File("/tmp/progui_demo.log").appendText(s + "\n");
    }

private:
    Skin current {};
};
