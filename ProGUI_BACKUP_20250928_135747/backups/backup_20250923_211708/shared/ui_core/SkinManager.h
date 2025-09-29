#pragma once
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <vector>

// Simple skin manager with PNG support. Falls back to vector draws when assets missing.
class SkinManager {
public:
    struct Skin {
        juce::String name { "Default" };
        // Assets (loaded if found)
        juce::Image knobImage;          // e.g. kit-07, knob-67, knob-70
        juce::Image meterVUFrame;       // VU frame/backplate
        juce::Image meterVUNeedle;      // optional needle image
        juce::Image meterLEDStrip;      // optional LED strip/sprite
        // Filmstrips (sequence of frames)
        std::vector<juce::Image> knobFrames;     // filmstrip frames for knob
        std::vector<juce::Image> meterVUFrames;  // filmstrip frames for VU
        std::vector<juce::Image> buttonFrames;   // [0]=normal, [1]=pressed
        std::vector<juce::Image> switchFrames;   // [0]=off, [1]=on

        // Theme overrides
        juce::Colour accent { 0xFF9B78FF };
        juce::Colour accent2{ 0xFF5AA8FF };
        juce::Colour panel  { 0xFF1F2632 };
        juce::Colour text   { 0xFFECEFF4 };
    };

    static SkinManager& instance() { static SkinManager mgr; return mgr; }

    // Attempts to load a skin from folder; scans for common asset names.
    bool loadSkin(const juce::File& folder) {
        if (!folder.isDirectory()) return false;
        Skin next = current; // keep colours
        next.name = folder.getFileName();
        auto tryLoad = [](const juce::File& f)->juce::Image {
            if (!f.existsAsFile()) return {};
            auto img = juce::ImageFileFormat::loadFrom(f);
            return img;
        };
        // Scan for assets (recursive), support common extensions and case variations
        juce::Array<juce::File> files;
        auto addMatches = [&](const char* pattern){
            auto matches = folder.findChildFiles(juce::File::findFiles, true, pattern);
            for (auto& f : matches) files.addIfNotAlreadyThere(f);
        };
        addMatches("*.png"); addMatches("*.PNG"); addMatches("*.jpg"); addMatches("*.jpeg"); addMatches("*.JPG"); addMatches("*.JPEG");
        auto folderName = folder.getFileName().toLowerCase();
        // Sort files by name to keep numeric order (001.png ...)
        struct FileNameComparator {
            int compareElements(const juce::File& a, const juce::File& b) const {
                return a.getFileName().compare(b.getFileName());
            }
        } comp;
        files.sort(comp, false);

        // If no explicit knobImage was found by name, choose the largest-area image as a candidate
        if (! next.knobImage.isValid())
        {
            int bestArea = 0;
            juce::Image bestImg;
            for (auto& f : files)
            {
                auto img = tryLoad(f);
                if (! img.isValid()) continue;
                int area = img.getWidth() * img.getHeight();
                if (area > bestArea) { bestArea = area; bestImg = img; }
            }
            if (bestImg.isValid()) next.knobImage = bestImg;
        }

        // Slice spritesheets into frames if needed (Fantasma-style)
        auto sliceSpriteStrip = [](const juce::Image& sheet, bool horizontal){
            std::vector<juce::Image> frames;
            if (! sheet.isValid()) return frames;
            int w = sheet.getWidth();
            int h = sheet.getHeight();
            if (horizontal)
            {
                if (h <= 0) return frames;
                int frameCount = w / h; // square frames laid out horizontally
                if (frameCount >= 8 && w % h == 0)
                {
                    for (int i = 0; i < frameCount; ++i)
                    {
                        auto sub = sheet.getClippedImage({ i*h, 0, h, h });
                        juce::Image f(juce::Image::ARGB, h, h, true);
                        juce::Graphics g(f);
                        g.drawImage(sub, juce::Rectangle<float>(0,0,(float)h,(float)h));
                        frames.push_back(f);
                    }
                }
            }
            else
            {
                if (w <= 0) return frames;
                int frameCount = h / w; // square frames laid out vertically
                if (frameCount >= 8 && h % w == 0)
                {
                    for (int i = 0; i < frameCount; ++i)
                    {
                        auto sub = sheet.getClippedImage({ 0, i*w, w, w });
                        juce::Image f(juce::Image::ARGB, w, w, true);
                        juce::Graphics g(f);
                        g.drawImage(sub, juce::Rectangle<float>(0,0,(float)w,(float)w));
                        frames.push_back(f);
                    }
                }
            }
            return frames;
        };
        if (next.knobFrames.empty() && next.knobImage.isValid())
        {
            auto horiz = sliceSpriteStrip(next.knobImage, true);
            auto vert  = sliceSpriteStrip(next.knobImage, false);
            auto pick = horiz.size() > vert.size() ? horiz : vert;
            if (pick.size() >= 8) { next.knobFrames = std::move(pick); }
        }
        if (next.meterVUFrames.empty() && next.meterVUFrame.isValid())
        {
            // Some VU sheets are horizontal or vertical sequences
            auto horiz = sliceSpriteStrip(next.meterVUFrame, true);
            auto vert  = sliceSpriteStrip(next.meterVUFrame, false);
            auto pick = horiz.size() > vert.size() ? horiz : vert;
            if (! pick.empty()) { next.meterVUFrames = std::move(pick); }
        }
        for (auto& f : files)
        {
            auto nm = f.getFileName().toLowerCase();
            // Primary filename-based heuristics
            if (! next.knobImage.isValid() && (nm == "knob.png" || nm.startsWith("knob") || nm.contains("kit-07") || nm.contains("knob-67") || nm.contains("knob-70")))
                next.knobImage = tryLoad(f);
            else if (! next.meterVUFrame.isValid() && (nm == "meter.png" || nm == "vu.png" || nm.contains("meter-vu") || nm.contains("vu")))
                next.meterVUFrame = tryLoad(f);
            else if (nm.contains("needle"))
                next.meterVUNeedle = tryLoad(f);
            else if (nm == "led_segment.png" || nm == "led_segment@2x.png" || nm.contains("led_segment"))
                next.meterLEDStrip = tryLoad(f);
            else if (nm.contains("led") || nm.contains("strip"))
                next.meterLEDStrip = tryLoad(f);
        }

        // Secondary heuristic: detect filmstrips by identical dimensions when names aren't descriptive
        if (next.knobFrames.empty())
        {
            std::map<std::pair<int,int>, std::vector<juce::Image>> groups;
            for (auto& f : files)
            {
                auto img = tryLoad(f);
                if (! img.isValid()) continue;
                auto key = std::make_pair(img.getWidth(), img.getHeight());
                groups[key].push_back(img);
            }
            // Choose the largest group as potential knob filmstrip
            size_t bestCount = 0; std::pair<int,int> bestKey {0,0};
            for (auto& kv : groups)
            {
                if (kv.second.size() > bestCount) { bestCount = kv.second.size(); bestKey = kv.first; }
            }
            if (bestCount >= 16) // reasonable number of frames
            {
                next.knobFrames = std::move(groups[bestKey]);
                next.knobImage = next.knobFrames.front();
            }
        }

        // (helper defined above)

        // Folder-name fallback heuristics: if folder suggests type but filenames are numeric (e.g., 001.png)
        if (! next.knobImage.isValid() && (folderName.contains("kit-07") || folderName.contains("knob-67") || folderName.contains("knob-70") || folderName.startsWith("knob")))
        {
            for (auto& f : files) { auto img = tryLoad(f); if (img.isValid()) { next.knobFrames.push_back(img); } }
            if (! next.knobFrames.empty()) next.knobImage = next.knobFrames.front();
        }
        if (! next.meterVUFrame.isValid() && (folderName.contains("vu") || folderName.contains("meter")))
        {
            for (auto& f : files) { auto img = tryLoad(f); if (img.isValid()) { next.meterVUFrames.push_back(img); } }
            if (! next.meterVUFrames.empty()) next.meterVUFrame = next.meterVUFrames.front();
        }
        // If no button/switch assets, generate simple glossy frames in-memory
        auto makeButtonFrame = [&](int w, int h, juce::Colour base, bool pressed){
            juce::Image img(juce::Image::ARGB, w, h, true);
            juce::Graphics g(img);
            auto r = juce::Rectangle<float>(0,0,(float)w,(float)h);
            juce::Colour top = pressed ? base.darker(0.15f) : base.brighter(0.10f);
            juce::Colour bot = pressed ? base.darker(0.35f) : base.darker(0.10f);
            g.setGradientFill({top, r.getX(), r.getY(), bot, r.getX(), r.getBottom(), false});
            g.fillRoundedRectangle(r, 6.0f);
            // inner gloss
            g.setColour(juce::Colours::white.withAlpha(pressed ? 0.05f : 0.12f));
            g.fillRoundedRectangle(r.withHeight(r.getHeight()*0.45f).reduced(2,1), 5.0f);
            // outline
            g.setColour(juce::Colours::black.withAlpha(0.35f));
            g.drawRoundedRectangle(r, 6.0f, 1.0f);
            return img;
        };
        auto makeSwitchFrame = [&](int w, int h, bool on, juce::Colour accent, juce::Colour panel){
            juce::Image img(juce::Image::ARGB, w, h, true);
            juce::Graphics g(img);
            auto rf = juce::Rectangle<float>(0,0,(float)w,(float)h);
            // body
            juce::Colour tTop = panel.brighter(0.12f);
            juce::Colour tBot = panel.darker(0.12f);
            g.setGradientFill({tTop, rf.getX(), rf.getY(), tBot, rf.getX(), rf.getBottom(), false});
            g.fillRoundedRectangle(rf, rf.getHeight()/2.0f);
            // knob
            auto knob = juce::Rectangle<float>(on ? rf.getWidth()-rf.getHeight() : 0.f, 0.f, rf.getHeight(), rf.getHeight());
            juce::Colour kTop = panel.brighter(0.18f);
            juce::Colour kBot = panel.darker(0.18f);
            g.setGradientFill({kTop, knob.getX(), knob.getY(), kBot, knob.getX(), knob.getBottom(), false});
            g.fillEllipse(knob);
            // glow when on
            if (on) {
                g.setColour(accent.withAlpha(0.30f));
                g.fillEllipse(knob.reduced(4));
            }
            // outline
            g.setColour(juce::Colours::black.withAlpha(0.25f));
            g.drawRoundedRectangle(rf, rf.getHeight()/2.0f, 1.0f);
            return img;
        };

        if (next.buttonFrames.empty()) {
            next.buttonFrames.push_back(makeButtonFrame(140, 28, next.panel, false));
            next.buttonFrames.push_back(makeButtonFrame(140, 28, next.panel, true));
        }
        if (next.switchFrames.empty()) {
            next.switchFrames.push_back(makeSwitchFrame(120, 28, false, next.accent, next.panel));
            next.switchFrames.push_back(makeSwitchFrame(120, 28, true, next.accent, next.panel));
        }

        current = std::move(next);
        logToFile("[SkinManager] Loaded skin from folder: " + folder.getFullPathName()
                  + ", knobImage=" + juce::String((int)current.knobImage.getWidth()) + "x" + juce::String((int)current.knobImage.getHeight())
                  + ", meterVUFrame=" + juce::String((int)current.meterVUFrame.getWidth()) + "x" + juce::String((int)current.meterVUFrame.getHeight())
                  + ", meterLEDStrip=" + juce::String((int)current.meterLEDStrip.getWidth()) + "x" + juce::String((int)current.meterLEDStrip.getHeight())
                  + ", filesFound=" + juce::String(files.size())
                  + ", knobFrames=" + juce::String((int)current.knobFrames.size())
                  + ", vuFrames=" + juce::String((int)current.meterVUFrames.size())
                  + ", buttonFrames=" + juce::String((int)current.buttonFrames.size())
                  + ", switchFrames=" + juce::String((int)current.switchFrames.size()));
        return true;
    }

    const Skin& skin() const { return current; }

    // Convenience: load known default skin folders
    bool loadDefaultProSkin() {
        juce::Array<juce::File> candidates {
            juce::File("/Volumes/MP 1/MoreMojo/Knobs/kit-07"),
            juce::File("/Volumes/MP 1/MoreMojo/Knobs/App Knobs folder/kit-07"),
            juce::File("/Volumes/MP 1/MoreMojo/Knobs/App Knobs folder/Pro/kit-07")
        };
        for (auto& f : candidates) if (f.isDirectory() && loadSkin(f)) return true;
        return false;
    }

    bool loadFantasmaAt(const juce::String& folderPath) {
        juce::File f(folderPath);
        if (! f.isDirectory()) return false;
        // Target Neptune knob_big specifically
        auto knobBig = f.getChildFile("Oneshots").getChildFile("knob_big");
        if (knobBig.isDirectory())
        {
            Skin next = current; // keep colours
            next.name = "Neptune";
            next.knobFrames.clear();
            auto files = knobBig.findChildFiles(juce::File::findFiles, false, "*.png");
            struct FileComparator {
                int compareElements(const juce::File& a, const juce::File& b) const {
                    return a.getFileName().compare(b.getFileName());
                }
            } comp;
            files.sort(comp, false);
            for (auto& file : files)
            {
                auto img = juce::ImageFileFormat::loadFrom(file);
                if (img.isValid()) next.knobFrames.push_back(img);
            }
            if (! next.knobFrames.empty())
            {
                next.knobImage = next.knobFrames.front();
                current = std::move(next);
                logToFile("[SkinManager] Loaded Neptune knob_big: " + juce::String((int)current.knobFrames.size()) + " frames");
                return true;
            }
        }
        return false;
    }
    bool loadDefaultConsumerSkinPrimary() { // knob-67
        juce::Array<juce::File> candidates {
            juce::File("/Volumes/MP 1/MoreMojo/Knobs/knob-67"),
            juce::File("/Volumes/MP 1/MoreMojo/Knobs/App Knobs folder/knob-67"),
            juce::File("/Volumes/MP 1/MoreMojo/Knobs/App Knobs folder/Consumer/knob-67")
        };
        for (auto& f : candidates) if (f.isDirectory() && loadSkin(f)) return true;
        return false;
    }
    bool loadDefaultConsumerSkinAlt() { // knob-70
        juce::Array<juce::File> candidates {
            juce::File("/Volumes/MP 1/MoreMojo/Knobs/knob-70"),
            juce::File("/Volumes/MP 1/MoreMojo/Knobs/App Knobs folder/knob-70"),
            juce::File("/Volumes/MP 1/MoreMojo/Knobs/App Knobs folder/Consumer/knob-70")
        };
        for (auto& f : candidates) if (f.isDirectory() && loadSkin(f)) return true;
        return false;
    }
    bool loadDefaultVUMeter() {
        juce::Array<juce::File> candidates {
            juce::File("/Volumes/MP 1/MoreMojo/Knobs/VU-Meter-04"),
            juce::File("/Volumes/MP 1/MoreMojo/Knobs/App Knobs folder/VU-meter/assets"),
            juce::File("/Volumes/MP 1/MoreMojo/Knobs/App Knobs folder/VU-meter"),
            juce::File("/Volumes/MP 1/MoreMojo/Knobs/App Knobs folder")
        };
        for (auto& f : candidates) if (f.exists()) { if (f.isDirectory() ? loadSkin(f) : false) return true; }
        return false;
    }

    static void logToFile(const juce::String& s) {
        juce::File("/tmp/progui_demo.log").appendText(s + "\n");
    }

private:
    Skin current {};
};
