#include "JUCEBridge.h"
#include <JuceHeader.h>

#include "../Source/SpectrumAnalyzer.h"
#include "../Source/ProEQComponent.h"
#include "../Source/ProCompressorComponent.h"

// Internal component wrapper class
class JUCEComponentWrapperImpl {
public:
    JUCEComponentWrapperImpl(JUCEComponentType type, float width, float height) {
        switch (type) {
            case kSpectrumAnalyzerComponent:
                component = std::make_unique<SpectrumAnalyzer>();
                break;
            case kProEQComponent:
                component = std::make_unique<ProEQComponent>();
                break;
            case kProCompressorComponent:
                component = std::make_unique<ProCompressorComponent>();
                break;
        }
        
        if (component != nullptr) {
            component->setSize(static_cast<int>(width), static_cast<int>(height));
            
            // Create NSView wrapper
            nsViewComponent = std::make_unique<juce::NSViewComponent>();
            nsViewComponent->setSize(static_cast<int>(width), static_cast<int>(height));
            nsViewComponent->addAndMakeVisible(component.get());
        }
    }
    
    ~JUCEComponentWrapperImpl() {
        // Clean up JUCE components
        component = nullptr;
        nsViewComponent = nullptr;
    }
    
    void resize(float width, float height) {
        if (component != nullptr) {
            component->setSize(static_cast<int>(width), static_cast<int>(height));
        }
        
        if (nsViewComponent != nullptr) {
            nsViewComponent->setSize(static_cast<int>(width), static_cast<int>(height));
        }
    }
    
    void setParameterValue(const char* parameterId, float value) {
        if (component == nullptr) return;
        
        // Handle different parameter types based on component
        juce::String paramId(parameterId);
        
        if (auto* eqComponent = dynamic_cast<ProEQComponent*>(component.get())) {
            if (paramId.startsWith("eq.band")) {
                // Parse band index and parameter type
                int bandIndex = paramId.substring(8, 9).getIntValue() - 1; // Extract the band number
                
                if (paramId.contains("gain")) {
                    eqComponent->setBandGain(bandIndex, value);
                } else if (paramId.contains("freq")) {
                    eqComponent->setBandFrequency(bandIndex, value);
                } else if (paramId.contains("q")) {
                    eqComponent->setBandQ(bandIndex, value);
                }
            }
        }
        else if (auto* compComponent = dynamic_cast<ProCompressorComponent*>(component.get())) {
            if (paramId == "comp.threshold") {
                compComponent->setThreshold(value);
            } else if (paramId == "comp.ratio") {
                compComponent->setRatio(value);
            } else if (paramId == "comp.attack") {
                compComponent->setAttack(value);
            } else if (paramId == "comp.release") {
                compComponent->setRelease(value);
            } else if (paramId == "comp.knee") {
                compComponent->setKnee(value);
            } else if (paramId == "comp.makeup") {
                compComponent->setMakeupGain(value);
            }
        }
    }
    
    float getParameterValue(const char* parameterId) {
        if (component == nullptr) return 0.0f;
        
        juce::String paramId(parameterId);
        
        if (auto* eqComponent = dynamic_cast<ProEQComponent*>(component.get())) {
            if (paramId.startsWith("eq.band")) {
                int bandIndex = paramId.substring(8, 9).getIntValue() - 1;
                
                if (paramId.contains("gain")) {
                    return eqComponent->getBandGain(bandIndex);
                } else if (paramId.contains("freq")) {
                    return eqComponent->getBandFrequency(bandIndex);
                } else if (paramId.contains("q")) {
                    return eqComponent->getBandQ(bandIndex);
                }
            }
        }
        else if (auto* compComponent = dynamic_cast<ProCompressorComponent*>(component.get())) {
            if (paramId == "comp.threshold") {
                return compComponent->getThreshold();
            } else if (paramId == "comp.ratio") {
                return compComponent->getRatio();
            } else if (paramId == "comp.attack") {
                return compComponent->getAttack();
            } else if (paramId == "comp.release") {
                return compComponent->getRelease();
            } else if (paramId == "comp.knee") {
                return compComponent->getKnee();
            } else if (paramId == "comp.makeup") {
                return compComponent->getMakeupGain();
            }
        }
        
        return 0.0f;
    }
    
    void processAudioBuffer(float** data, int numChannels, int numSamples) {
        if (component == nullptr) return;
        
        // Create a JUCE AudioBuffer from the raw data
        juce::AudioBuffer<float> buffer(data, numChannels, numSamples);
        
        if (auto* analyzer = dynamic_cast<SpectrumAnalyzer*>(component.get())) {
            analyzer->processAudio(buffer);
        }
        else if (auto* eqComponent = dynamic_cast<ProEQComponent*>(component.get())) {
            eqComponent->processAudio(buffer);
        }
        else if (auto* compComponent = dynamic_cast<ProCompressorComponent*>(component.get())) {
            compComponent->processAudio(buffer);
        }
    }
    
    void* getNativeView() {
        if (nsViewComponent != nullptr) {
            return (void*)nsViewComponent->getView();
        }
        return nullptr;
    }
    
    void attachToParentView(void* parentView) {
        if (nsViewComponent != nullptr) {
            nsViewComponent->setView(parentView);
        }
    }
    
private:
    std::unique_ptr<juce::Component> component;
    std::unique_ptr<juce::NSViewComponent> nsViewComponent;
};

// C-style structs for opaque interface
struct JUCEComponentWrapper {
    JUCEComponentWrapperImpl* impl;
};

struct AudioBuffer {
    float** data;
    int numChannels;
    int numSamples;
};

// C API implementation
JUCEComponentWrapper* CreateJUCEComponent(JUCEComponentType type, float width, float height) {
    auto* wrapper = new JUCEComponentWrapper;
    wrapper->impl = new JUCEComponentWrapperImpl(type, width, height);
    return wrapper;
}

void DestroyJUCEComponent(JUCEComponentWrapper* component) {
    if (component != nullptr) {
        delete component->impl;
        delete component;
    }
}

void ResizeJUCEComponent(JUCEComponentWrapper* component, float width, float height) {
    if (component != nullptr && component->impl != nullptr) {
        component->impl->resize(width, height);
    }
}

void SetParameterValue(JUCEComponentWrapper* component, const char* parameterId, float value) {
    if (component != nullptr && component->impl != nullptr) {
        component->impl->setParameterValue(parameterId, value);
    }
}

float GetParameterValue(JUCEComponentWrapper* component, const char* parameterId) {
    if (component != nullptr && component->impl != nullptr) {
        return component->impl->getParameterValue(parameterId);
    }
    return 0.0f;
}

AudioBuffer* CreateAudioBuffer(int numChannels, int numSamples) {
    auto* buffer = new AudioBuffer;
    
    buffer->numChannels = numChannels;
    buffer->numSamples = numSamples;
    
    buffer->data = new float*[numChannels];
    for (int i = 0; i < numChannels; ++i) {
        buffer->data[i] = new float[numSamples]();  // Initialize to zero
    }
    
    return buffer;
}

void DestroyAudioBuffer(AudioBuffer* buffer) {
    if (buffer != nullptr) {
        for (int i = 0; i < buffer->numChannels; ++i) {
            delete[] buffer->data[i];
        }
        delete[] buffer->data;
        delete buffer;
    }
}

void SetAudioBufferData(AudioBuffer* buffer, int channel, int index, float value) {
    if (buffer != nullptr && channel >= 0 && channel < buffer->numChannels && 
        index >= 0 && index < buffer->numSamples) {
        buffer->data[channel][index] = value;
    }
}

void ProcessAudioBuffer(JUCEComponentWrapper* component, AudioBuffer* buffer) {
    if (component != nullptr && component->impl != nullptr && buffer != nullptr) {
        component->impl->processAudioBuffer(buffer->data, buffer->numChannels, buffer->numSamples);
    }
}

void* GetNativeView(JUCEComponentWrapper* component) {
    if (component != nullptr && component->impl != nullptr) {
        return component->impl->getNativeView();
    }
    return nullptr;
}

void AttachToParentView(JUCEComponentWrapper* component, void* parentView) {
    if (component != nullptr && component->impl != nullptr) {
        component->impl->attachToParentView(parentView);
    }
}
