#pragma once
#include "GLCommon.h"
#include <vector>

namespace mojoGL {

/**
 * ResidualSpectrum - Visualizes the residual spectrum from null tests
 * Shows the spectral content of what's changed in the processing
 */
class ResidualSpectrum : public GLComponent {
public:
    ResidualSpectrum() : 
        fftSize(2048),
        spectrumData(fftSize / 2, 0.0f),
        residualType(ResidualType::Null)
    {}
    
    // Enum for different residual visualization modes
    enum class ResidualType {
        Null,       // True null test (A - B after alignment)
        Difference, // Simple A - B difference
        ABRatio     // A/B ratio (spectral difference)
    };
    
    // Set the spectrum data
    void setSpectrumData(const std::vector<float>& data) {
        juce::ScopedLock lock(dataMutex);
        
        if (data.empty()) return;
        
        // Resize data if needed
        if (data.size() != spectrumData.size()) {
            spectrumData.resize(data.size(), 0.0f);
        }
        
        // Copy data
        std::copy(data.begin(), data.end(), spectrumData.begin());
        
        dataChanged = true;
    }
    
    // Set the residual type
    void setResidualType(ResidualType type) {
        residualType = type;
        dataChanged = true;
    }
    
    // Set frequency range for display
    void setFrequencyRange(float minFreq, float maxFreq) {
        minFrequency = minFreq;
        maxFrequency = maxFreq;
        dataChanged = true;
    }
    
    // Set amplitude range for display (dB)
    void setAmplitudeRange(float minDb, float maxDb) {
        minDecibels = minDb;
        maxDecibels = maxDb;
        dataChanged = true;
    }
    
    // Get residual RMS level
    float getResidualRMS() const {
        return residualRMS;
    }
    
    // Get spectral tilt (brightness/darkness)
    float getSpectralTilt() const {
        return spectralTilt;
    }
    
    // Get crest factor (peakiness)
    float getCrestFactor() const {
        return crestFactor;
    }
    
protected:
    // OpenGL setup
    void setupGL() override {
        // Load shaders
        if (!loadShaders()) {
            // Handle shader loading error
            juce::Logger::writeToLog("Failed to load ResidualSpectrum shaders: " + shader.getErrorMessage());
            return;
        }
        
        // Create vertex buffer
        glGenBuffers(1, &vertexBuffer);
        
        // Set up initial vertex data
        updateVertexBuffer();
    }
    
    // OpenGL drawing
    void paintGL() override {
        // Check if data has changed and update buffer if needed
        if (dataChanged) {
            updateVertexBuffer();
            dataChanged = false;
        }
        
        // Clear background
        juce::OpenGLHelpers::clear(juce::Colour(24, 24, 32));
        
        // Use shader
        shader.use();
        
        // Set spectrum color based on residual type
        juce::Colour spectrumColor;
        switch (residualType) {
            case ResidualType::Null:
                spectrumColor = juce::Colour(100, 180, 255); // Blue
                break;
            case ResidualType::Difference:
                spectrumColor = juce::Colour(255, 120, 80); // Orange
                break;
            case ResidualType::ABRatio:
                spectrumColor = juce::Colour(180, 120, 255); // Purple
                break;
        }
        
        shader.setUniform("lineColor", spectrumColor.getFloatRed(), 
                         spectrumColor.getFloatGreen(), 
                         spectrumColor.getFloatBlue(), 
                         0.9f);
        
        // Set up line drawing
        glLineWidth(2.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Draw spectrum
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);
        
        const int vertexCount = static_cast<int>(spectrumData.size());
        glDrawArrays(GL_LINE_STRIP, 0, vertexCount);
        
        glDisableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // Draw grid and labels
        drawGridAndLabels();
    }
    
    // OpenGL cleanup
    void cleanupGL() override {
        // Delete buffer
        if (vertexBuffer != 0) {
            glDeleteBuffers(1, &vertexBuffer);
            vertexBuffer = 0;
        }
    }
    
private:
    // Load shaders
    bool loadShaders() {
        const juce::String vertexShaderSource = R"(
            #version 330 core
            layout (location = 0) in vec2 position;
            
            void main() {
                gl_Position = vec4(position, 0.0, 1.0);
            }
        )";
        
        const juce::String fragmentShaderSource = R"(
            #version 330 core
            out vec4 FragColor;
            
            uniform vec4 lineColor;
            
            void main() {
                FragColor = lineColor;
            }
        )";
        
        return shader.compile(vertexShaderSource, fragmentShaderSource);
    }
    
    // Update vertex buffer with spectrum data
    void updateVertexBuffer() {
        juce::ScopedLock lock(dataMutex);
        
        const int numBins = static_cast<int>(spectrumData.size());
        if (numBins <= 0) return;
        
        // Analyze the spectrum data
        analyzeSpectrum();
        
        // Create vertex data
        std::vector<float> vertices(numBins * 2);
        
        for (int i = 0; i < numBins; ++i) {
            // X coordinate (frequency) from -1 to 1 with logarithmic scaling
            const float binFreq = i * sampleRate / (2.0f * numBins);
            const float normX = std::log10(binFreq / minFrequency) / std::log10(maxFrequency / minFrequency);
            vertices[i * 2] = -1.0f + 2.0f * juce::jlimit(0.0f, 1.0f, normX);
            
            // Y coordinate (amplitude) from -1 to 1
            const float dbValue = 20.0f * std::log10(spectrumData[i] + 0.0000001f);
            const float normY = (dbValue - minDecibels) / (maxDecibels - minDecibels);
            vertices[i * 2 + 1] = -1.0f + 2.0f * juce::jlimit(0.0f, 1.0f, normY);
        }
        
        // Upload vertex data
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    // Draw grid and labels
    void drawGridAndLabels() {
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
        
        // Calculate layout
        const float margin = 30.0f;
        const float width = static_cast<float>(viewport[2]);
        const float height = static_cast<float>(viewport[3]);
        const float chartLeft = margin;
        const float chartTop = margin;
        const float chartWidth = width - 2 * margin;
        const float chartHeight = height - 2 * margin;
        const float chartBottom = chartTop + chartHeight;
        
        // Draw title
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        
        juce::String title;
        switch (residualType) {
            case ResidualType::Null:
                title = "Residual/Null Spectrum";
                break;
            case ResidualType::Difference:
                title = "Difference Spectrum";
                break;
            case ResidualType::ABRatio:
                title = "A/B Ratio Spectrum";
                break;
        }
        
        g.drawText(title, chartLeft, 5.0f, chartWidth, 20.0f, juce::Justification::centred);
        
        // Draw grid
        g.setColour(juce::Colours::grey.withAlpha(0.4f));
        
        // Frequency grid lines (logarithmic)
        std::array<float, 8> freqPoints = {20.0f, 50.0f, 100.0f, 500.0f, 1000.0f, 5000.0f, 10000.0f, 20000.0f};
        
        for (float freq : freqPoints) {
            if (freq < minFrequency || freq > maxFrequency) continue;
            
            const float normX = std::log10(freq / minFrequency) / std::log10(maxFrequency / minFrequency);
            const float x = chartLeft + normX * chartWidth;
            
            g.drawLine(x, chartTop, x, chartBottom, 1.0f);
            
            // Format frequency label
            juce::String label;
            if (freq >= 1000.0f) {
                label = juce::String(freq / 1000.0f) + "k";
            } else {
                label = juce::String(static_cast<int>(freq));
            }
            
            g.drawText(label, x - 15.0f, chartBottom + 5.0f, 30.0f, 20.0f, juce::Justification::centred);
        }
        
        // Amplitude grid lines (linear in dB)
        const int dbStep = 12;
        for (int db = static_cast<int>(minDecibels); db <= static_cast<int>(maxDecibels); db += dbStep) {
            const float normY = (db - minDecibels) / (maxDecibels - minDecibels);
            const float y = chartBottom - normY * chartHeight;
            
            g.drawLine(chartLeft, y, chartLeft + chartWidth, y, 1.0f);
            
            g.drawText(juce::String(db) + " dB", chartLeft - 25.0f, y - 10.0f, 20.0f, 20.0f, juce::Justification::right);
        }
        
        // Draw frame
        g.drawRect(juce::Rectangle<float>(chartLeft, chartTop, chartWidth, chartHeight), 1.0f);
        
        // Draw axis labels
        g.setFont(juce::Font(12.0f));
        g.drawText("Frequency (Hz)", chartLeft, chartBottom + 25.0f, chartWidth, 20.0f, juce::Justification::centred);
        
        g.addTransform(juce::AffineTransform::rotation(-juce::MathConstants<float>::halfPi, 15.0f, height / 2.0f));
        g.drawText("Amplitude (dB)", 15.0f - height / 2.0f, 0.0f, height, 30.0f, juce::Justification::centred);
        
        // Draw metrics
        g.resetToDefaultState();
        g.setFont(juce::Font(12.0f));
        
        const float metricsX = chartLeft + 10.0f;
        const float metricsY = chartTop + 10.0f;
        const float lineHeight = 18.0f;
        
        g.drawText("RMS: " + juce::String(residualRMS, 1) + " dB", 
                  metricsX, metricsY, 120.0f, 20.0f, juce::Justification::left);
                  
        g.drawText("Tilt: " + juce::String(spectralTilt, 1) + " dB/oct", 
                  metricsX, metricsY + lineHeight, 120.0f, 20.0f, juce::Justification::left);
                  
        g.drawText("Crest: " + juce::String(crestFactor, 1) + " dB", 
                  metricsX, metricsY + lineHeight * 2.0f, 120.0f, 20.0f, juce::Justification::left);
        
        // Convert to texture
        juce::OpenGLTexture labelTexture;
        labelTexture.loadImage(tempImage);
        
        // Draw texture
        juce::OpenGLHelpers::drawQuad2D(0.0f, 0.0f, width, height, 
                                       juce::Parallelogram<float>({0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}), 
                                       labelTexture, 1.0f);
    }
    
    // Analyze spectrum data to calculate metrics
    void analyzeSpectrum() {
        const int numBins = static_cast<int>(spectrumData.size());
        if (numBins <= 0) return;
        
        // Calculate RMS level
        float sumSquares = 0.0f;
        float peakValue = 0.0f;
        
        for (float value : spectrumData) {
            sumSquares += value * value;
            peakValue = std::max(peakValue, value);
        }
        
        const float rms = std::sqrt(sumSquares / static_cast<float>(numBins));
        residualRMS = 20.0f * std::log10(rms + 0.0000001f);
        
        // Calculate crest factor (peak to RMS ratio)
        crestFactor = 20.0f * std::log10((peakValue / rms) + 0.0000001f);
        
        // Calculate spectral tilt (high vs low frequency balance)
        const int lowBand = numBins / 8;    // 1/8 of spectrum (low frequencies)
        const int highBand = numBins * 3/4; // 3/4 of spectrum (high frequencies)
        
        float lowSum = 0.0f;
        float highSum = 0.0f;
        
        for (int i = 0; i < lowBand; ++i) {
            lowSum += spectrumData[i];
        }
        
        for (int i = highBand; i < numBins; ++i) {
            highSum += spectrumData[i];
        }
        
        const float lowAvg = lowSum / static_cast<float>(lowBand);
        const float highAvg = highSum / static_cast<float>(numBins - highBand);
        
        if (lowAvg > 0.0f && highAvg > 0.0f) {
            spectralTilt = 20.0f * std::log10(highAvg / lowAvg);
        } else {
            spectralTilt = 0.0f;
        }
    }
    
    // Instance variables
    int fftSize;
    std::vector<float> spectrumData;
    ResidualType residualType;
    bool dataChanged = true;
    
    float minFrequency = 20.0f;
    float maxFrequency = 20000.0f;
    float minDecibels = -96.0f;
    float maxDecibels = 0.0f;
    float sampleRate = 44100.0f;
    
    float residualRMS = -120.0f;
    float spectralTilt = 0.0f;
    float crestFactor = 0.0f;
    
    ShaderProgram shader;
    GLuint vertexBuffer = 0;
    
    juce::CriticalSection dataMutex;
};

} // namespace mojoGL
