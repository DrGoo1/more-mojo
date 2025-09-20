#pragma once
#include "GLCommon.h"
#include <vector>
#include <deque>

namespace mojoGL {

/**
 * TEI Visualization - Transient Edge Integrity metrics visualization
 * 
 * Shows rise time, pre-ring energy, and overshoot differences between
 * original and processed audio for transients
 */
class TEIVisualization : public GLComponent {
public:
    // Struct for TEI metrics data
    struct TEIMetrics {
        float deltaRiseTime;   // Change in rise time (µs)
        float preRingEnergy;   // Pre-ring energy (dB)
        float overshootPercent; // Overshoot (%)
        
        // Calculate overall score (0-1)
        float getScore() const {
            // Lower values are better for all metrics
            const float riseTimeScore = std::exp(-std::abs(deltaRiseTime) / 500.0f);
            const float preRingScore = std::exp(-std::abs(preRingEnergy) / 20.0f);
            const float overshootScore = std::exp(-std::abs(overshootPercent) / 20.0f);
            
            return (riseTimeScore * 0.4f + preRingScore * 0.4f + overshootScore * 0.2f);
        }
    };
    
    // Struct for transient waveform data
    struct TransientWaveform {
        std::vector<float> originalSamples;
        std::vector<float> processedSamples;
        int transientIndex;  // Sample index where transient occurs
    };
    
    TEIVisualization() : maxTransients(5) {}
    
    // Add new TEI metrics for a detected transient
    void addTransientMetrics(const TEIMetrics& metrics, const TransientWaveform& waveform) {
        juce::ScopedLock lock(dataMutex);
        
        // Add to deque (latest at front)
        metricsHistory.push_front(metrics);
        waveformHistory.push_front(waveform);
        
        // Maintain max size
        while (metricsHistory.size() > maxTransients) {
            metricsHistory.pop_back();
            waveformHistory.pop_back();
        }
        
        // Update overall score
        updateOverallScore();
        
        dataChanged = true;
    }
    
    // Get the current overall TEI score (0-1)
    float getOverallScore() const {
        return overallScore;
    }
    
    // Set the maximum number of transients to display
    void setMaxTransients(int max) {
        juce::ScopedLock lock(dataMutex);
        
        maxTransients = juce::jmax(1, max);
        
        // Trim history if needed
        while (metricsHistory.size() > maxTransients) {
            metricsHistory.pop_back();
            waveformHistory.pop_back();
        }
        
        dataChanged = true;
    }
    
protected:
    // OpenGL setup
    void setupGL() override {
        // Load shaders
        if (!loadShaders()) {
            // Handle shader loading error
            juce::Logger::writeToLog("Failed to load TEI visualization shaders: " + shader.getErrorMessage());
            return;
        }
        
        // Create vertex buffer
        glGenBuffers(1, &vertexBuffer);
        
        // Set up initial vertex data
        updateVertexBuffers();
    }
    
    // OpenGL drawing
    void paintGL() override {
        // Check if data has changed and update buffers if needed
        if (dataChanged) {
            updateVertexBuffers();
            dataChanged = false;
        }
        
        // Clear background
        juce::OpenGLHelpers::clear(juce::Colour(24, 24, 32));
        
        // Use shader
        shader.use();
        
        // Set up line drawing
        glLineWidth(2.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Draw waveforms
        drawWaveforms();
        
        // Draw metrics table
        drawMetricsTable();
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
    
    // Update vertex buffers with current data
    void updateVertexBuffers() {
        // No vertex buffer updates needed for TEI visualization
        // We'll use immediate mode for the dynamic content
    }
    
    // Draw waveform comparison
    void drawWaveforms() {
        juce::ScopedLock lock(dataMutex);
        
        if (waveformHistory.empty()) {
            return;
        }
        
        // Get viewport dimensions
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        
        // Calculate layout
        const int waveformHeight = viewport[3] / (maxTransients + 1);
        const int waveformWidth = viewport[2] - 200; // Leave space for metrics
        
        // Draw each waveform (latest first)
        for (size_t i = 0; i < waveformHistory.size(); ++i) {
            const auto& waveform = waveformHistory[i];
            
            // Y position for this waveform
            const int yOffset = i * waveformHeight;
            
            drawSingleWaveform(waveform, 10, yOffset, waveformWidth, waveformHeight);
        }
    }
    
    // Draw a single waveform comparison
    void drawSingleWaveform(const TransientWaveform& waveform, int x, int y, int width, int height) {
        if (waveform.originalSamples.empty() || waveform.processedSamples.empty()) {
            return;
        }
        
        // Set up 2D drawing
        juce::OpenGLHelpers::setTextureMode(juce::OpenGLHelpers::TextureMode::normal);
        juce::OpenGLGraphicsContextCustomShader::setMatrixMode(juce::OpenGLGraphicsContextCustomShader::MatrixMode::projection);
        juce::OpenGLGraphicsContextCustomShader::loadIdentity();
        juce::OpenGLGraphicsContextCustomShader::ortho(0, getWidth(), getHeight(), 0, -1, 1);
        juce::OpenGLGraphicsContextCustomShader::setMatrixMode(juce::OpenGLGraphicsContextCustomShader::MatrixMode::modelview);
        juce::OpenGLGraphicsContextCustomShader::loadIdentity();
        
        // Create a temporary Graphics object for drawing
        juce::Image tempImage(juce::Image::ARGB, getWidth(), getHeight(), true);
        juce::Graphics g(tempImage);
        
        // Draw waveform border
        g.setColour(juce::Colours::grey.withAlpha(0.3f));
        g.drawRect(x, y, width, height, 1);
        
        // Draw zero line
        g.setColour(juce::Colours::grey.withAlpha(0.5f));
        g.drawLine(x, y + height / 2, x + width, y + height / 2, 1.0f);
        
        // Draw transient marker (vertical line)
        g.setColour(juce::Colours::yellow.withAlpha(0.7f));
        int transientX = x + width / 2;
        g.drawLine(transientX, y, transientX, y + height, 1.0f);
        
        // Get max amplitude for scaling
        float maxAmp = 0.001f; // Small non-zero value to avoid division by zero
        
        for (float sample : waveform.originalSamples) {
            maxAmp = std::max(maxAmp, std::abs(sample));
        }
        
        for (float sample : waveform.processedSamples) {
            maxAmp = std::max(maxAmp, std::abs(sample));
        }
        
        // Scale factor for drawing
        const float scale = height / (2.0f * maxAmp);
        
        // Draw original waveform (blue)
        g.setColour(juce::Colour(80, 180, 255));
        
        juce::Path originalPath;
        const int numSamples = static_cast<int>(waveform.originalSamples.size());
        
        // Calculate sample spacing
        const float xStep = static_cast<float>(width) / static_cast<float>(numSamples);
        
        // Initialize the path
        originalPath.startNewSubPath(x, y + height / 2);
        
        for (int i = 0; i < numSamples; ++i) {
            const float xPos = x + i * xStep;
            const float yPos = y + height / 2 - waveform.originalSamples[i] * scale;
            originalPath.lineTo(xPos, yPos);
        }
        
        // Draw path
        g.strokePath(originalPath, juce::PathStrokeType(1.5f));
        
        // Draw processed waveform (orange)
        g.setColour(juce::Colour(255, 120, 80));
        
        juce::Path processedPath;
        const int numProcessedSamples = static_cast<int>(waveform.processedSamples.size());
        
        // Initialize the path
        processedPath.startNewSubPath(x, y + height / 2);
        
        for (int i = 0; i < numProcessedSamples; ++i) {
            const float xPos = x + i * xStep;
            const float yPos = y + height / 2 - waveform.processedSamples[i] * scale;
            processedPath.lineTo(xPos, yPos);
        }
        
        // Draw path
        g.strokePath(processedPath, juce::PathStrokeType(1.5f));
        
        // Draw legend
        g.setColour(juce::Colour(80, 180, 255));
        g.drawLine(x + 10, y + 15, x + 30, y + 15, 1.5f);
        g.setColour(juce::Colours::white);
        g.drawText("Original", x + 35, y + 5, 60, 20, juce::Justification::centredLeft);
        
        g.setColour(juce::Colour(255, 120, 80));
        g.drawLine(x + 100, y + 15, x + 120, y + 15, 1.5f);
        g.setColour(juce::Colours::white);
        g.drawText("Processed", x + 125, y + 5, 70, 20, juce::Justification::centredLeft);
        
        // Convert to texture
        juce::OpenGLTexture waveformTexture;
        waveformTexture.loadImage(tempImage);
        
        // Draw texture
        juce::OpenGLHelpers::drawQuad2D(0.0f, 0.0f, static_cast<float>(getWidth()), static_cast<float>(getHeight()), 
                                       juce::Parallelogram<float>({0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}), 
                                       waveformTexture, 1.0f);
    }
    
    // Draw metrics table
    void drawMetricsTable() {
        juce::ScopedLock lock(dataMutex);
        
        if (metricsHistory.empty()) {
            return;
        }
        
        // Set up 2D drawing
        juce::OpenGLHelpers::setTextureMode(juce::OpenGLHelpers::TextureMode::normal);
        juce::OpenGLGraphicsContextCustomShader::setMatrixMode(juce::OpenGLGraphicsContextCustomShader::MatrixMode::projection);
        juce::OpenGLGraphicsContextCustomShader::loadIdentity();
        juce::OpenGLGraphicsContextCustomShader::ortho(0, getWidth(), getHeight(), 0, -1, 1);
        juce::OpenGLGraphicsContextCustomShader::setMatrixMode(juce::OpenGLGraphicsContextCustomShader::MatrixMode::modelview);
        juce::OpenGLGraphicsContextCustomShader::loadIdentity();
        
        // Create a temporary Graphics object for drawing
        juce::Image tempImage(juce::Image::ARGB, getWidth(), getHeight(), true);
        juce::Graphics g(tempImage);
        
        // Calculate layout
        const int tableX = getWidth() - 190;
        const int tableY = 50;
        const int rowHeight = 25;
        
        // Draw table header
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawText("TEI Metrics", tableX, tableY - 30, 180, 25, juce::Justification::centred);
        
        g.setFont(juce::Font(12.0f));
        g.drawText("ΔRise (µs)", tableX, tableY, 60, 20, juce::Justification::centred);
        g.drawText("Pre-ring (dB)", tableX + 60, tableY, 70, 20, juce::Justification::centred);
        g.drawText("Overshoot (%)", tableX + 130, tableY, 60, 20, juce::Justification::centred);
        
        // Draw horizontal divider
        g.drawLine(tableX, tableY + 22, tableX + 190, tableY + 22, 1.0f);
        
        // Draw metrics for each transient
        g.setFont(juce::Font(11.0f));
        
        for (size_t i = 0; i < metricsHistory.size(); ++i) {
            const auto& metrics = metricsHistory[i];
            const int rowY = tableY + 25 + static_cast<int>(i) * rowHeight;
            
            // Set row color based on index (latest = white, older = more transparent)
            const float alpha = 1.0f - 0.2f * static_cast<float>(i);
            g.setColour(juce::Colours::white.withAlpha(alpha));
            
            // Format and draw metrics
            g.drawText(juce::String(static_cast<int>(metrics.deltaRiseTime)), 
                      tableX, rowY, 60, 20, juce::Justification::centred);
                      
            g.drawText(juce::String(metrics.preRingEnergy, 1), 
                      tableX + 60, rowY, 70, 20, juce::Justification::centred);
                      
            g.drawText(juce::String(metrics.overshootPercent, 1), 
                      tableX + 130, rowY, 60, 20, juce::Justification::centred);
        }
        
        // Draw overall score
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.setColour(getScoreColor(overallScore));
        g.drawText("Score: " + juce::String(static_cast<int>(overallScore * 100)), 
                  tableX, tableY + 25 + static_cast<int>(metricsHistory.size()) * rowHeight + 10, 
                  190, 25, juce::Justification::centred);
        
        // Convert to texture
        juce::OpenGLTexture tableTexture;
        tableTexture.loadImage(tempImage);
        
        // Draw texture
        juce::OpenGLHelpers::drawQuad2D(0.0f, 0.0f, static_cast<float>(getWidth()), static_cast<float>(getHeight()), 
                                       juce::Parallelogram<float>({0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}), 
                                       tableTexture, 1.0f);
    }
    
    // Get a color based on score (red to green gradient)
    juce::Colour getScoreColor(float score) const {
        // Red to yellow to green
        if (score < 0.5f) {
            // Red to yellow
            return juce::Colour::fromHSV(0.15f * score * 2.0f, 0.9f, 0.9f, 1.0f);
        } else {
            // Yellow to green
            return juce::Colour::fromHSV(0.15f + 0.25f * (score - 0.5f) * 2.0f, 0.9f, 0.9f, 1.0f);
        }
    }
    
    // Calculate overall score from history
    void updateOverallScore() {
        if (metricsHistory.empty()) {
            overallScore = 0.5f; // Default neutral score
            return;
        }
        
        // Weight recent transients more heavily
        float weightedSum = 0.0f;
        float weightSum = 0.0f;
        
        for (size_t i = 0; i < metricsHistory.size(); ++i) {
            const float weight = 1.0f / (1.0f + static_cast<float>(i) * 0.5f); // More weight to recent transients
            weightedSum += metricsHistory[i].getScore() * weight;
            weightSum += weight;
        }
        
        overallScore = weightedSum / weightSum;
    }
    
    // Instance variables
    int maxTransients;
    std::deque<TEIMetrics> metricsHistory;
    std::deque<TransientWaveform> waveformHistory;
    float overallScore = 0.5f;
    bool dataChanged = true;
    
    ShaderProgram shader;
    GLuint vertexBuffer = 0;
    
    juce::CriticalSection dataMutex;
};

} // namespace mojoGL
