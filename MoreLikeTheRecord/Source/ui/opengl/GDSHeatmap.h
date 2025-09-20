#pragma once
#include "GLCommon.h"
#include <vector>

namespace mojoGL {

/**
 * GDSHeatmap - Group Delay Spread visualization using OpenGL
 * Displays frequency vs. group delay with a color-coded heatmap
 */
class GDSHeatmap : public GLComponent {
public:
    GDSHeatmap() : data(256 * 128, 0.0f), width(256), height(128) {}
    
    // Set heatmap data
    void setData(const std::vector<float>& newData, int w, int h) {
        if (w <= 0 || h <= 0 || newData.empty()) return;
        
        // Copy data
        juce::ScopedLock lock(dataMutex);
        width = w;
        height = h;
        data = newData;
        
        // Make sure data size matches dimensions
        if (static_cast<int>(data.size()) != width * height) {
            data.resize(width * height, 0.0f);
        }
        
        dataChanged = true;
    }
    
    // Set color map for visualization
    void setColorMap(ColorMap map) {
        colorMap = map;
        dataChanged = true;
    }
    
    // Set frequency range for labeling
    void setFrequencyRange(float minFreq, float maxFreq) {
        minFrequency = minFreq;
        maxFrequency = maxFreq;
    }
    
    // Set group delay range for labeling
    void setGroupDelayRange(float minDelay, float maxDelay) {
        minGroupDelay = minDelay;
        maxGroupDelay = maxDelay;
    }
    
    // Get the data range
    std::pair<float, float> getDataRange() const {
        return {dataMin, dataMax};
    }
    
    // Enumerate available color maps
    enum class ColorMap {
        Viridis,   // Blue-green-yellow
        Magma,     // Purple-red-yellow
        Inferno,   // Black-red-yellow
        Plasma,    // Purple-orange-yellow
        Cividis    // Blue-yellow (colorblind-friendly)
    };
    
protected:
    // OpenGL setup
    void setupGL() override {
        // Load shaders
        if (!loadShaders()) {
            // Handle shader loading error
            juce::Logger::writeToLog("Failed to load GDS heatmap shaders: " + shader.getErrorMessage());
            return;
        }
        
        // Create texture
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        // Create buffer objects
        glGenBuffers(1, &vertexBuffer);
        glGenBuffers(1, &indexBuffer);
        
        // Set up vertex data for a quad
        const GLfloat vertices[] = {
            -1.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f, 0.0f, 1.0f  // top-left
        };
        
        const GLuint indices[] = {
            0, 1, 2, // first triangle
            2, 3, 0  // second triangle
        };
        
        // Upload vertex data
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        
        // Upload index data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        
        // Unbind buffers
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        
        // Initialize texture data
        updateTexture();
    }
    
    // OpenGL drawing
    void paintGL() override {
        // Check if data has changed and update texture if needed
        if (dataChanged) {
            updateTexture();
            dataChanged = false;
        }
        
        // Clear background
        juce::OpenGLHelpers::clear(juce::Colour(0, 0, 0, 0));
        
        // Use shader
        shader.use();
        
        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        shader.setUniform("textureSampler", 0);
        
        // Set colormap
        shader.setUniform("colorMap", static_cast<int>(colorMap));
        
        // Set up vertex attributes
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        
        // Position attribute (x, y)
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Texture coordinate attribute (u, v)
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
        
        // Draw quad
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // Disable attributes
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        
        // Unbind buffers
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        
        // Draw labels
        drawLabels();
    }
    
    // OpenGL cleanup
    void cleanupGL() override {
        // Delete texture
        if (textureID != 0) {
            glDeleteTextures(1, &textureID);
            textureID = 0;
        }
        
        // Delete buffers
        if (vertexBuffer != 0) {
            glDeleteBuffers(1, &vertexBuffer);
            vertexBuffer = 0;
        }
        
        if (indexBuffer != 0) {
            glDeleteBuffers(1, &indexBuffer);
            indexBuffer = 0;
        }
    }
    
private:
    // Load shaders
    bool loadShaders() {
        const juce::String vertexShaderSource = R"(
            #version 330 core
            layout (location = 0) in vec2 position;
            layout (location = 1) in vec2 texCoord;
            
            out vec2 TexCoord;
            
            void main() {
                gl_Position = vec4(position, 0.0, 1.0);
                TexCoord = texCoord;
            }
        )";
        
        const juce::String fragmentShaderSource = R"(
            #version 330 core
            in vec2 TexCoord;
            out vec4 FragColor;
            
            uniform sampler2D textureSampler;
            uniform int colorMap;
            
            vec3 viridis(float t) {
                const vec3 c0 = vec3(0.2777273272234177, 0.005407344544966578, 0.3340998053353061);
                const vec3 c1 = vec3(0.1050930431085774, 0.5348623495668753, 0.7456397718949003);
                const vec3 c2 = vec3(0.8661667052745518, 0.8627919428166982, 0.1305180466718456);
                return c0 + t * c1 + t * t * c2;
            }
            
            vec3 magma(float t) {
                const vec3 c0 = vec3(0.001462, 0.000466, 0.013866);
                const vec3 c1 = vec3(0.826532, 0.060893, 0.892168);
                const vec3 c2 = vec3(0.971832, 0.880873, 0.505714);
                return c0 + t * c1 + t * t * c2;
            }
            
            vec3 inferno(float t) {
                const vec3 c0 = vec3(0.001462, 0.000466, 0.013866);
                const vec3 c1 = vec3(0.873412, 0.236209, 0.348752);
                const vec3 c2 = vec3(0.983868, 0.904867, 0.136897);
                return c0 + t * c1 + t * t * c2;
            }
            
            vec3 plasma(float t) {
                const vec3 c0 = vec3(0.050383, 0.029803, 0.527975);
                const vec3 c1 = vec3(1.259729, 0.546826, 0.000837);
                const vec3 c2 = vec3(-0.314713, 0.225662, 0.383695);
                return c0 + t * c1 + t * t * c2;
            }
            
            vec3 cividis(float t) {
                const vec3 c0 = vec3(0.0, 0.3, 0.4);
                const vec3 c1 = vec3(0.0, 0.3, 0.1);
                const vec3 c2 = vec3(1.0, 0.6, -0.5);
                return c0 + t * c1 + t * t * c2;
            }
            
            void main() {
                float value = texture(textureSampler, TexCoord).r;
                
                vec3 color;
                switch (colorMap) {
                    case 0: color = viridis(value); break;
                    case 1: color = magma(value); break;
                    case 2: color = inferno(value); break;
                    case 3: color = plasma(value); break;
                    case 4: color = cividis(value); break;
                    default: color = viridis(value);
                }
                
                // Apply alpha based on value
                float alpha = value > 0.01 ? 1.0 : value * 100.0;
                FragColor = vec4(color, alpha);
            }
        )";
        
        return shader.compile(vertexShaderSource, fragmentShaderSource);
    }
    
    // Update texture data
    void updateTexture() {
        juce::ScopedLock lock(dataMutex);
        
        // Find data range
        dataMin = 1.0f;
        dataMax = 0.0f;
        
        for (float value : data) {
            if (value > 0.0f) {
                dataMin = std::min(dataMin, value);
                dataMax = std::max(dataMax, value);
            }
        }
        
        // Normalize data to 0-1 range
        std::vector<float> normalized(data.size());
        for (size_t i = 0; i < data.size(); ++i) {
            if (data[i] > 0.0f && dataMax > dataMin) {
                normalized[i] = (data[i] - dataMin) / (dataMax - dataMin);
            } else {
                normalized[i] = 0.0f;
            }
        }
        
        // Update texture
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, normalized.data());
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    // Draw frequency and group delay labels
    void drawLabels() {
        juce::OpenGLHelpers::setTextureMode(juce::OpenGLHelpers::TextureMode::normal);
        
        // Get current viewport
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        
        // Set up projection matrix for 2D drawing
        juce::OpenGLGraphicsContextCustomShader::setMatrixMode(juce::OpenGLGraphicsContextCustomShader::MatrixMode::projection);
        juce::OpenGLGraphicsContextCustomShader::loadIdentity();
        juce::OpenGLGraphicsContextCustomShader::ortho(0, viewport[2], viewport[3], 0, -1, 1);
        
        // Set up modelview matrix
        juce::OpenGLGraphicsContextCustomShader::setMatrixMode(juce::OpenGLGraphicsContextCustomShader::MatrixMode::modelview);
        juce::OpenGLGraphicsContextCustomShader::loadIdentity();
        
        // Create a temporary Graphics object for drawing
        juce::Image tempImage(juce::Image::ARGB, viewport[2], viewport[3], true);
        juce::Graphics g(tempImage);
        
        // Draw frequency labels (x-axis)
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(12.0f));
        
        // Draw frequency ticks at octave intervals
        const float margin = 20.0f;
        const float chartWidth = viewport[2] - 2 * margin;
        const float chartHeight = viewport[3] - 2 * margin;
        const float chartBottom = viewport[3] - margin;
        
        // Draw frequency octave labels (20Hz, 100Hz, 1kHz, etc.)
        std::array<float, 8> freqPoints = {20.0f, 50.0f, 100.0f, 500.0f, 1000.0f, 5000.0f, 10000.0f, 20000.0f};
        
        for (float freq : freqPoints) {
            // Calculate x position (logarithmic)
            const float normX = std::log10(freq / minFrequency) / std::log10(maxFrequency / minFrequency);
            const float x = margin + normX * chartWidth;
            
            // Format frequency label
            juce::String label;
            if (freq >= 1000.0f) {
                label = juce::String(freq / 1000.0f) + "k";
            } else {
                label = juce::String(static_cast<int>(freq));
            }
            
            // Draw tick and label
            g.drawLine(x, chartBottom, x, chartBottom - 5.0f, 1.0f);
            g.drawText(label, juce::Rectangle<float>(x - 15.0f, chartBottom, 30.0f, 15.0f),
                      juce::Justification::centredTop, true);
        }
        
        // Draw group delay labels (y-axis)
        std::array<float, 5> delayPoints = {0.0f, 0.5f, 1.0f, 2.0f, 5.0f};
        
        for (float delay : delayPoints) {
            // Calculate y position (linear)
            const float normY = 1.0f - (delay - minGroupDelay) / (maxGroupDelay - minGroupDelay);
            const float y = margin + normY * chartHeight;
            
            // Format delay label (ms)
            juce::String label = juce::String(delay) + " ms";
            
            // Draw tick and label
            g.drawLine(margin, y, margin + 5.0f, y, 1.0f);
            g.drawText(label, juce::Rectangle<float>(margin - 50.0f, y - 7.0f, 45.0f, 15.0f),
                      juce::Justification::centredRight, true);
        }
        
        // Draw axis labels
        g.drawText("Frequency (Hz)", juce::Rectangle<float>(margin, chartBottom + 15.0f, chartWidth, 20.0f),
                  juce::Justification::centred, true);
        
        g.drawText("Group Delay (ms)", juce::Rectangle<float>(margin - 55.0f, margin, 50.0f, chartHeight),
                  juce::Justification::centred, true);
        
        // Draw title
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawText("Group Delay Spread (GDS)", juce::Rectangle<float>(margin, margin - 25.0f, chartWidth, 20.0f),
                  juce::Justification::centred, true);
        
        // Convert to texture
        juce::OpenGLTexture labelTexture;
        labelTexture.loadImage(tempImage);
        
        // Draw texture
        juce::OpenGLHelpers::drawQuad2D(0.0f, 0.0f, static_cast<float>(viewport[2]), static_cast<float>(viewport[3]), 
                                       juce::Parallelogram<float>({0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}), 
                                       labelTexture, 1.0f);
    }
    
    // Instance variables
    std::vector<float> data;
    int width, height;
    float dataMin = 0.0f, dataMax = 1.0f;
    bool dataChanged = true;
    
    float minFrequency = 20.0f;
    float maxFrequency = 20000.0f;
    float minGroupDelay = 0.0f;
    float maxGroupDelay = 5.0f;
    
    ColorMap colorMap = ColorMap::Viridis;
    
    ShaderProgram shader;
    GLuint textureID = 0;
    GLuint vertexBuffer = 0;
    GLuint indexBuffer = 0;
    
    juce::CriticalSection dataMutex;
};

} // namespace mojoGL
