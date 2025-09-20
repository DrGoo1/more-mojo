#pragma once
#include "../../JUCE/modules/juce_gui_basics/juce_gui_basics.h"

// Fix for JUCE notification observer memory issue
namespace moremojo {
    // Custom initialization to be run before JUCE startup
    void prepareJuceInit()
    {
        // Pre-initialize critical objects to avoid memory issues
        juce::ScopedJuceInitialiser_GUI scopedJuce;
    }
}
