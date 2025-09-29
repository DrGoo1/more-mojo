#include "SubwindowManager.h"
#include "ProcessSubwindow.h"
#include "ISPSubwindow.h"
#include "SRCSubwindow.h"
#include "JitterSubwindow.h"
#include "AlignSubwindow.h"
#include "TransientSubwindow.h"
#include "DeEsserSubwindow.h"
#include "MLARSubwindow.h"
#include "TransformerSubwindow.h"

SubwindowManager::SubwindowManager()
{
}

SubwindowManager::~SubwindowManager()
{
    closeAllSubwindows();
}

void SubwindowManager::openSubwindow(ProcessType processType, juce::Component* parent)
{
    // Close existing window if open
    closeSubwindow(processType);
    
    // Create new subwindow content
    auto content = createSubwindowContent(processType);
    if (!content)
        return;
    
    // Create wrapper window
    auto windowName = getProcessName(processType) + " Controls";
    auto wrapper = std::make_unique<SubwindowWrapper>(windowName, std::move(content));
    
    // Set window properties
    wrapper->setResizable(true, true);
    wrapper->setUsingNativeTitleBar(true);
    wrapper->centreWithSize(800, 600);
    wrapper->setVisible(true);
    wrapper->toFront(true);
    
    // Store in map
    openWindows[processType] = std::move(wrapper);
}

void SubwindowManager::closeSubwindow(ProcessType processType)
{
    auto it = openWindows.find(processType);
    if (it != openWindows.end())
    {
        it->second.reset();
        openWindows.erase(it);
    }
}

void SubwindowManager::closeAllSubwindows()
{
    openWindows.clear();
}

bool SubwindowManager::isSubwindowOpen(ProcessType processType) const
{
    return openWindows.find(processType) != openWindows.end();
}

juce::String SubwindowManager::getProcessName(ProcessType processType)
{
    switch (processType)
    {
        case ISP:        return "Intersample Processing";
        case SRC:        return "Sample Rate Conversion";
        case JITTER:     return "Jitter & Accumulation";
        case ALIGN:      return "Phase Alignment";
        case TRANSIENT:  return "Transient Shaping";
        case DEESSER:    return "De-essing";
        case MLAR:       return "More Like A Record";
        case TRANSFORMER: return "Transformer";
        default:         return "Unknown Process";
    }
}

std::unique_ptr<ProcessSubwindow> SubwindowManager::createSubwindowContent(ProcessType processType)
{
    switch (processType)
    {
        case ISP:        return std::make_unique<ISPSubwindow>();
        case SRC:        return std::make_unique<SRCSubwindow>();
        case JITTER:     return std::make_unique<JitterSubwindow>();
        case ALIGN:      return std::make_unique<AlignSubwindow>();
        case TRANSIENT:  return std::make_unique<TransientSubwindow>();
        case DEESSER:    return std::make_unique<DeEsserSubwindow>();
        case MLAR:       return std::make_unique<MLARSubwindow>();
        case TRANSFORMER: return std::make_unique<TransformerSubwindow>();
        default:         return nullptr;
    }
}

// SubwindowWrapper implementation
SubwindowManager::SubwindowWrapper::SubwindowWrapper(const juce::String& name, std::unique_ptr<ProcessSubwindow> content)
    : juce::DocumentWindow(name, juce::Colour(0xFF1a1a2e), juce::DocumentWindow::allButtons),
      contentComponent(std::move(content))
{
    setContentOwned(contentComponent.release(), true);
}

SubwindowManager::SubwindowWrapper::~SubwindowWrapper()
{
}

void SubwindowManager::SubwindowWrapper::closeButtonPressed()
{
    setVisible(false);
}
