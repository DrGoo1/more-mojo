#pragma once
#include "GLCommon.h"

namespace mojoGL {

/**
 * HeatmapGL - Texture-backed heatmap visualization where data 0..1 maps to blue→red
 * Used for visualizing GDS and other time-frequency data
 */
class HeatmapGL : public GLComp {
public:
    HeatmapGL() = default;
    ~HeatmapGL() override {
        // Clean up texture if allocated
        if (texture != 0) {
            glDeleteTextures(1, &texture);
            texture = 0;
        }
    }
    
    /**
     * Set heatmap data and dimensions
     * 
     * @param d Data array (values 0.0-1.0)
     * @param w Width of the heatmap grid
     * @param h Height of the heatmap grid
     */
    void setData(std::vector<float> d, int w, int h) {
        data = std::move(d);
        width = w;
        height = h;
        repaint();
    }
    
    /**
     * OpenGL rendering
     */
    void paintGL() override {
        // Ensure we have valid data
        if (width <= 0 || height <= 0 || data.empty()) return;
        
        // Create an image for the heatmap
        juce::Image img(juce::Image::ARGB, width, height, true);
        juce::Graphics g(img);
        
        // Draw each pixel with color mapped from data value
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float value = data[(size_t)y * width + x];
                value = juce::jlimit(0.0f, 1.0f, value);
                
                // Map value (0-1) to color (blue to red through green/yellow)
                juce::Colour color = juce::Colour::fromHSV(0.66f - 0.66f * value, 0.85f, 0.95f, 1.0f);
                
                g.setColour(color);
                g.fillRect(x, y, 1, 1);
            }
        }
        
        // Clear OpenGL context
        juce::OpenGLHelpers::clear(juce::Colours::transparentBlack);
        
        // Create texture if needed
        if (texture == 0) {
            glGenTextures(1, &texture);
        }
        
        // Copy image to OpenGL texture and display it
        juce::OpenGLContext::getCurrentContext()->copyTexture(texture, img, 0, 0, 0);
        
        // Draw texture to fill the component
        auto bounds = getLocalBounds().toFloat();
        juce::OpenGLHelpers::drawQuad2D(
            bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
            juce::Parallelogram<float>({0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}),
            texture, 1.0f);
    }
    
private:
    std::vector<float> data;
    int width{0}, height{0};
    juce::GLuint texture{0};
};

} // namespace mojoGL
