#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>
#include <unordered_map>

// Forward declarations
class ProcessSubwindow;
class ISPSubwindow;
class SRCSubwindow;
class JitterSubwindow;
class AlignSubwindow;
class TransientSubwindow;
class DeEsserSubwindow;
class MLARSubwindow;
class TransformerSubwindow;

/**
 * Manages all process subwindows - opening, closing, positioning
 */
class SubwindowManager
{
public:
    enum ProcessType
    {
        ISP = 0,
        SRC,
        JITTER,
        ALIGN,
        TRANSIENT,
        DEESSER,
        MLAR,
        TRANSFORMER
    };

    SubwindowManager();
    ~SubwindowManager();

    // Open a specific process subwindow
    void openSubwindow(ProcessType processType, juce::Component* parent);
    
    // Close a specific subwindow
    void closeSubwindow(ProcessType processType);
    
    // Close all subwindows
    void closeAllSubwindows();
    
    // Check if a subwindow is open
    bool isSubwindowOpen(ProcessType processType) const;
    
    // Get process name from type
    static juce::String getProcessName(ProcessType processType);

private:
    // Window class for each subwindow
    class SubwindowWrapper : public juce::DocumentWindow
    {
    public:
        SubwindowWrapper(const juce::String& name, std::unique_ptr<ProcessSubwindow> content);
        ~SubwindowWrapper() override;
        
        void closeButtonPressed() override;
        
    private:
        std::unique_ptr<ProcessSubwindow> contentComponent;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SubwindowWrapper)
    };

    // Create subwindow content based on type
    std::unique_ptr<ProcessSubwindow> createSubwindowContent(ProcessType processType);
    
    // Map of open subwindows
    std::unordered_map<ProcessType, std::unique_ptr<SubwindowWrapper>> openWindows;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SubwindowManager)
};
