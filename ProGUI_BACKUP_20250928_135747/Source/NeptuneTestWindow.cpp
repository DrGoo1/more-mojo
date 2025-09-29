#include "NeptuneTestWindow.h"
#include "Neptune/NeptuneKnobDemo.h"

using namespace juce;

// Adjust these to your actual directories. You said the big knob frames are here:
static const char* kBigPath   = "/Volumes/MP 1/MoreMojo/NeptuneUI/03 Delivery/VST/Assets/Animation/Oneshots/knob_big/";
// If you only have big for now, you can point med/small to same folder or create symlinks:
static const char* kMedPath   = "/Volumes/MP 1/MoreMojo/NeptuneUI/03 Delivery/VST/Assets/Animation/Oneshots/knob_big/";
static const char* kSmallPath = "/Volumes/MP 1/MoreMojo/NeptuneUI/03 Delivery/VST/Assets/Animation/Oneshots/knob_big/";

NeptuneTestWindow::NeptuneTestWindow()
: DocumentWindow ("Neptune Test Window",
                  Colours::darkgrey,
                  DocumentWindow::closeButton)
{
    setUsingNativeTitleBar(true);
    setResizable(true, true);
    centreWithSize(520, 360);

    File big (kBigPath), med (kMedPath), small (kSmallPath);
    content = std::make_unique<NeptuneKnobDemo>(big, med, small);

    setContentOwned(content.get(), true); // window takes ownership of Component
    content.release();                    // prevent double-delete, DocumentWindow owns it now

    setVisible(true);
}

NeptuneTestWindow::~NeptuneTestWindow() = default;

void NeptuneTestWindow::closeButtonPressed()
{
    setVisible(false);
    // If used from an on-demand button, the owner can delete this (keep a unique_ptr in owner)
}
