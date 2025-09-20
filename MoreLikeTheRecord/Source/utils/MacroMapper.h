#pragma once
#include <JuceHeader.h>
#include <functional>
#include <vector>

namespace mojo {

/**
 * MacroMapper - Maps a single macro parameter to multiple module parameters
 * with customizable curves and ranges
 */
class MacroMapper : public juce::AudioProcessorValueTreeState::Listener {
public:
    // Mapping curve type
    enum class CurveType {
        Linear,     // Linear mapping
        SoftKnee,   // Soft knee (ease-in-ease-out)
        Logarithmic, // Logarithmic (slower at start, faster at end)
        Exponential, // Exponential (faster at start, slower at end)
        SCurve      // S-curve (very slow at extremes, fast in middle)
    };
    
    // Struct for a parameter mapping
    struct Mapping {
        juce::String paramId;      // Target parameter ID
        float startValue;          // Value at macro = 0
        float endValue;            // Value at macro = 1
        float startPosition;       // Macro position (0-1) where this starts to take effect
        float endPosition;         // Macro position (0-1) where this reaches full effect
        CurveType curveType;       // Type of mapping curve
        
        Mapping(const juce::String& id, float start, float end, 
                float startPos = 0.0f, float endPos = 1.0f,
                CurveType curve = CurveType::SoftKnee)
            : paramId(id), startValue(start), endValue(end),
              startPosition(startPos), endPosition(endPos), curveType(curve) {}
    };
    
    MacroMapper() = default;
    
    // Initialize with parent AudioProcessorValueTreeState
    void init(juce::AudioProcessorValueTreeState& apvts, const juce::String& macroParamId) {
        this->apvts = &apvts;
        this->macroParamId = macroParamId;
        
        // Add listener for macro parameter changes
        apvts.addParameterListener(macroParamId, this);
    }
    
    // Clean up
    void cleanup() {
        if (apvts) {
            apvts->removeParameterListener(macroParamId, this);
        }
    }
    
    // Add a parameter mapping
    void addMapping(const Mapping& mapping) {
        mappings.push_back(mapping);
    }
    
    // Add a parameter mapping (convenience method)
    void addMapping(const juce::String& paramId, float start, float end, 
                    float startPos = 0.0f, float endPos = 1.0f,
                    CurveType curve = CurveType::SoftKnee) {
        addMapping(Mapping(paramId, start, end, startPos, endPos, curve));
    }
    
    // Clear all mappings
    void clearMappings() {
        mappings.clear();
    }
    
    // Parameter changed callback
    void parameterChanged(const juce::String& parameterID, float newValue) override {
        if (parameterID == macroParamId) {
            updateMappings(newValue);
        }
    }
    
    // Update all mappings with the current macro value
    void updateMappings(float macroValue) {
        // Ensure macro is normalized to 0-1 range
        const float normalizedMacro = juce::jlimit(0.0f, 1.0f, macroValue);
        
        // Update each parameter based on its mapping
        for (const auto& mapping : mappings) {
            // Calculate normalized position in the mapping's range
            float normPos = 0.0f;
            if (mapping.endPosition > mapping.startPosition) {
                normPos = (normalizedMacro - mapping.startPosition) / 
                          (mapping.endPosition - mapping.startPosition);
                normPos = juce::jlimit(0.0f, 1.0f, normPos);
            }
            
            // Apply curve
            const float curvedPos = applyCurve(normPos, mapping.curveType);
            
            // Calculate mapped value
            const float mappedValue = mapping.startValue + 
                                     curvedPos * (mapping.endValue - mapping.startValue);
            
            // Update parameter
            if (auto* param = apvts->getParameter(mapping.paramId)) {
                // Convert to normalized value for the parameter
                const float normValue = param->convertTo0to1(mappedValue);
                param->setValueNotifyingHost(normValue);
            }
        }
    }
    
    // Force an update of all mappings with the current macro value
    void forceMappingUpdate() {
        if (apvts && auto* param = apvts->getParameter(macroParamId)) {
            // Convert from normalized value
            const float macroValue = param->convertFrom0to1(param->getValue());
            updateMappings(macroValue);
        }
    }
    
private:
    // Apply curve to a normalized (0-1) value
    float applyCurve(float normValue, CurveType curveType) const {
        switch (curveType) {
            case CurveType::Linear:
                return normValue;
                
            case CurveType::SoftKnee:
                // Soft knee with ease-in-ease-out (smoothstep)
                return normValue * normValue * (3.0f - 2.0f * normValue);
                
            case CurveType::Logarithmic:
                // Logarithmic curve (slower start, faster end)
                if (normValue <= 0.0f) return 0.0f;
                return std::log10(1.0f + 9.0f * normValue) / std::log10(10.0f);
                
            case CurveType::Exponential:
                // Exponential curve (faster start, slower end)
                return 1.0f - std::exp(-3.0f * normValue);
                
            case CurveType::SCurve:
                // S-curve (very slow at extremes, fast in middle)
                return 0.5f - 0.5f * std::cos(juce::MathConstants<float>::pi * normValue);
                
            default:
                return normValue;
        }
    }
    
    // Instance variables
    juce::AudioProcessorValueTreeState* apvts = nullptr;
    juce::String macroParamId;
    std::vector<Mapping> mappings;
};

/**
 * MoreLikeTheRecordMapper - Specific MacroMapper for the "More Like The Record" macro
 * 
 * Maps the main consumer-facing macro to all underlying processing modules
 * with carefully tuned curves and ranges
 */
class MoreLikeTheRecordMapper : public MacroMapper {
public:
    MoreLikeTheRecordMapper() = default;
    
    // Initialize with standard "More Like The Record" mappings
    void initWithDefaultMappings(juce::AudioProcessorValueTreeState& apvts) {
        // Initialize the mapper
        init(apvts, "macro");
        
        // Clear any existing mappings
        clearMappings();
        
        // Add mappings for each module
        
        // 1. TimingSafeResampler (enables early, gradually increases strength)
        addMapping("resampler_on", 0.0f, 1.0f, 0.0f, 0.1f, CurveType::SoftKnee);
        addMapping("resampler_oversampling", 2.0f, 4.0f, 0.1f, 0.5f, CurveType::SoftKnee);
        addMapping("resampler_isp_guard", 0.0f, 1.0f, 0.0f, 0.1f, CurveType::SoftKnee);
        addMapping("resampler_interpolation", 0.0f, 1.0f, 0.1f, 0.3f, CurveType::SoftKnee);
        
        // 2. TransientDetect (enables in mid-range, gradually increases sensitivity)
        addMapping("transient_on", 0.0f, 1.0f, 0.2f, 0.4f, CurveType::SoftKnee);
        addMapping("transient_bands", 6.0f, 8.0f, 0.3f, 0.7f, CurveType::Logarithmic);
        addMapping("transient_sensitivity", 50.0f, 85.0f, 0.3f, 0.7f, CurveType::SoftKnee);
        addMapping("transient_hysteresis", 50.0f, 30.0f, 0.3f, 0.7f, CurveType::SoftKnee); // Decreases with macro increase
        
        // 3. FractionalDelayAlign (enables in mid-range, increases strength towards higher values)
        addMapping("align_on", 0.0f, 1.0f, 0.3f, 0.5f, CurveType::SoftKnee);
        addMapping("align_max_correction", 0.3f, 0.6f, 0.4f, 0.8f, CurveType::SoftKnee);
        addMapping("align_width_protect", 70.0f, 30.0f, 0.4f, 0.9f, CurveType::Logarithmic); // Decreases with macro increase
        
        // 4. AnalogContinuity (enables only at higher values, subtle increase)
        addMapping("analog_on", 0.0f, 1.0f, 0.6f, 0.7f, CurveType::SoftKnee);
        addMapping("analog_mix", 0.0f, 30.0f, 0.6f, 1.0f, CurveType::SCurve);
        
        // 5. Limiter + LevelMatch (always enabled, ceiling becomes stricter at higher settings)
        addMapping("limiter_lookahead", 0.5f, 1.0f, 0.0f, 0.8f, CurveType::SoftKnee);
        addMapping("limiter_ceiling", 0.0f, -0.3f, 0.0f, 1.0f, CurveType::Logarithmic); // Decreases with macro increase
    }
};

} // namespace mojo
