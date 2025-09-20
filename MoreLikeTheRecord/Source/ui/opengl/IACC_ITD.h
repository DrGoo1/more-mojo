#pragma once
#include "GLCommon.h"
#include <vector>

namespace mojoGL {

/**
 * IACC_ITD - Visualization for Interaural Coherence and Time Difference
 * Displays IACC over time with ITD variance indicators
 */
class IACC_ITD : public GLComponent {
public:
    IACC_ITD() : 
        historySize(240),
        iaccHistory(historySize, 0.5f),
        itdHistory(historySize, 0.0f),
        itdVariance(0.0f),
        averageIACC(0.5f)
    {
        // Initialize colors
        colors.iaccLine = juce::Colour(80, 180, 255);    // Blue for IACC
        colors.itdLine = juce::Colour(255, 120, 80);     // Orange for ITD
        colors.grid = juce::Colours::grey.withAlpha(0.4f);
        colors.text = juce::Colours::white;
        colors.background = juce::Colour(24, 24, 32);
    }
    
    // Add new IACC and ITD values to history
    void addValues(float iacc, float itdMs) {
        juce::ScopedLock lock(dataMutex);
        
        // Shift history
        for (int i = 0; i < historySize - 1; ++i) {
            iaccHistory[i] = iaccHistory[i + 1];
            itdHistory[i] = itdHistory[i + 1];
        }
        
        // Add new values
        iaccHistory[historySize - 1] = juce::jlimit(0.0f, 1.0f, iacc);
        itdHistory[historySize - 1] = itdMs;
        
        // Calculate ITD variance
        calculateStatistics();
        
        dataChanged = true;
    }
    
    // Set the history size
    void setHistorySize(int newSize) {
        juce::ScopedLock lock(dataMutex);
        
        historySize = juce::jmax(10, newSize);
        iaccHistory.resize(historySize, 0.5f);
        itdHistory.resize(historySize, 0.0f);
        
        dataChanged = true;
    }
    
    // Get current statistics
    float getITDVariance() const { return itdVariance; }
    float getAverageIACC() const { return averageIACC; }
    
protected:
    // OpenGL setup
    void setupGL() override {
        // Load shaders
        if (!loadShaders()) {
            // Handle shader loading error
            juce::Logger::writeToLog("Failed to load IACC/ITD shaders: " + shader.getErrorMessage());
            return;
        }
        
        // Create vertex buffers
        glGenBuffers(1, &iaccVertexBuffer);
        glGenBuffers(1, &itdVertexBuffer);
        
        // Set up vertex data
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
        juce::OpenGLHelpers::clear(colors.background);
        
        // Use shader
        shader.use();
        
        // Draw grid
        drawGrid();
        
        // Set up line drawing
        glLineWidth(2.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Draw ITD line
        shader.setUniform("lineColor", colors.itdLine.getFloatRed(), 
                         colors.itdLine.getFloatGreen(), 
                         colors.itdLine.getFloatBlue(), 
                         colors.itdLine.getFloatAlpha());
        
        glBindBuffer(GL_ARRAY_BUFFER, itdVertexBuffer);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_LINE_STRIP, 0, historySize);
        glDisableVertexAttribArray(0);
        
        // Draw IACC line
        shader.setUniform("lineColor", colors.iaccLine.getFloatRed(), 
                         colors.iaccLine.getFloatGreen(), 
                         colors.iaccLine.getFloatBlue(), 
                         colors.iaccLine.getFloatAlpha());
        
        glBindBuffer(GL_ARRAY_BUFFER, iaccVertexBuffer);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_LINE_STRIP, 0, historySize);
        glDisableVertexAttribArray(0);
        
        // Unbind buffers
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // Draw labels
        drawLabels();
    }
    
    // OpenGL cleanup
    void cleanupGL() override {
        // Delete buffers
        if (iaccVertexBuffer != 0) {
            glDeleteBuffers(1, &iaccVertexBuffer);
            iaccVertexBuffer = 0;
        }
        
        if (itdVertexBuffer != 0) {
            glDeleteBuffers(1, &itdVertexBuffer);
            itdVertexBuffer = 0;
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
        juce::ScopedLock lock(dataMutex);
        
        // Create vertex data for IACC line
        std::vector<float> iaccVertices(historySize * 2);
        for (int i = 0; i < historySize; ++i) {
            // X coordinate (time) from -1 to 1
            iaccVertices[i * 2] = -1.0f + 2.0f * static_cast<float>(i) / static_cast<float>(historySize - 1);
            // Y coordinate (IACC) from -1 to 1
            iaccVertices[i * 2 + 1] = -1.0f + 2.0f * iaccHistory[i];
        }
        
        // Create vertex data for ITD line
        std::vector<float> itdVertices(historySize * 2);
        
        // Find ITD range for normalization
        float maxITD = 0.01f; // Minimum range to avoid division by zero
        for (float itd : itdHistory) {
            maxITD = std::max(maxITD, std::abs(itd));
        }
        
        for (int i = 0; i < historySize; ++i) {
            // X coordinate (time) from -1 to 1
            itdVertices[i * 2] = -1.0f + 2.0f * static_cast<float>(i) / static_cast<float>(historySize - 1);
            // Y coordinate (ITD) from -1 to 1, scaled
            itdVertices[i * 2 + 1] = juce::jlimit(-0.9f, 0.9f, itdHistory[i] / maxITD);
        }
        
        // Upload IACC vertices
        glBindBuffer(GL_ARRAY_BUFFER, iaccVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, iaccVertices.size() * sizeof(float), iaccVertices.data(), GL_DYNAMIC_DRAW);
        
        // Upload ITD vertices
        glBindBuffer(GL_ARRAY_BUFFER, itdVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, itdVertices.size() * sizeof(float), itdVertices.data(), GL_DYNAMIC_DRAW);
        
        // Unbind buffer
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    // Draw grid lines
    void drawGrid() {
        // Set grid color
        shader.setUniform("lineColor", colors.grid.getFloatRed(), 
                         colors.grid.getFloatGreen(), 
                         colors.grid.getFloatBlue(), 
                         colors.grid.getFloatAlpha());
        
        // Draw horizontal lines (IACC values at 0.0, 0.5, 1.0)
        const float horizLines[] = {
            -1.0f, -1.0f,  1.0f, -1.0f,  // Bottom (IACC = 0.0)
            -1.0f,  0.0f,  1.0f,  0.0f,  // Middle (IACC = 0.5)
            -1.0f,  1.0f,  1.0f,  1.0f   // Top (IACC = 1.0)
        };
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, horizLines);
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_LINES, 0, 6);
        
        // Draw vertical lines (time divisions)
        const float vertLines[] = {
            -1.0f, -1.0f, -1.0f,  1.0f,  // Left edge
            -0.5f, -1.0f, -0.5f,  1.0f,  // 25% mark
             0.0f, -1.0f,  0.0f,  1.0f,  // Center
             0.5f, -1.0f,  0.5f,  1.0f,  // 75% mark
             1.0f, -1.0f,  1.0f,  1.0f   // Right edge
        };
        
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertLines);
        glDrawArrays(GL_LINES, 0, 10);
        
        glDisableVertexAttribArray(0);
    }
    
    // Draw labels
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
        
        // Draw labels
        g.setColour(colors.text);
        g.setFont(juce::Font(12.0f));
        
        const float margin = 10.0f;
        const float width = static_cast<float>(viewport[2]);
        const float height = static_cast<float>(viewport[3]);
        
        // IACC labels (0.0, 0.5, 1.0)
        g.drawText("1.0", margin, margin, 25.0f, 20.0f, juce::Justification::centredLeft, true);
        g.drawText("0.5", margin, height / 2.0f - 10.0f, 25.0f, 20.0f, juce::Justification::centredLeft, true);
        g.drawText("0.0", margin, height - margin - 20.0f, 25.0f, 20.0f, juce::Justification::centredLeft, true);
        
        // Time labels
        const float secondsPerDiv = 0.1f; // Assume 10 divisions for a total of 1 second
        g.drawText("0s", width - margin - 20.0f, height - margin - 20.0f, 25.0f, 20.0f, juce::Justification::centredRight, true);
        g.drawText("-" + juce::String(secondsPerDiv * 10) + "s", margin, height - margin - 20.0f, 40.0f, 20.0f, juce::Justification::centredLeft, true);
        
        // Legend
        const float legendX = width - margin - 120.0f;
        const float legendY = margin;
        
        // IACC legend
        g.setColour(colors.iaccLine);
        g.drawLine(legendX, legendY + 10.0f, legendX + 20.0f, legendY + 10.0f, 2.0f);
        g.setColour(colors.text);
        g.drawText("IACC: " + juce::String(averageIACC, 2), legendX + 25.0f, legendY, 90.0f, 20.0f, juce::Justification::centredLeft, true);
        
        // ITD legend
        g.setColour(colors.itdLine);
        g.drawLine(legendX, legendY + 30.0f, legendX + 20.0f, legendY + 30.0f, 2.0f);
        g.setColour(colors.text);
        g.drawText("ITD σ: " + juce::String(itdVariance, 2) + " ms", legendX + 25.0f, legendY + 20.0f, 90.0f, 20.0f, juce::Justification::centredLeft, true);
        
        // Title
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawText("Interaural Coherence & Time", width / 2.0f - 100.0f, margin, 200.0f, 20.0f, juce::Justification::centred, true);
        
        // Convert to texture
        juce::OpenGLTexture labelTexture;
        labelTexture.loadImage(tempImage);
        
        // Draw texture
        juce::OpenGLHelpers::drawQuad2D(0.0f, 0.0f, width, height, 
                                       juce::Parallelogram<float>({0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}), 
                                       labelTexture, 1.0f);
    }
    
    // Calculate statistics (ITD variance, average IACC)
    void calculateStatistics() {
        // Calculate ITD variance
        float sumITD = 0.0f;
        float sumITDSq = 0.0f;
        
        for (float itd : itdHistory) {
            sumITD += itd;
            sumITDSq += itd * itd;
        }
        
        float meanITD = sumITD / static_cast<float>(historySize);
        itdVariance = std::sqrt(sumITDSq / static_cast<float>(historySize) - meanITD * meanITD);
        
        // Calculate average IACC
        float sumIACC = 0.0f;
        for (float iacc : iaccHistory) {
            sumIACC += iacc;
        }
        
        averageIACC = sumIACC / static_cast<float>(historySize);
    }
    
    // Instance variables
    struct Colors {
        juce::Colour iaccLine;
        juce::Colour itdLine;
        juce::Colour grid;
        juce::Colour text;
        juce::Colour background;
    } colors;
    
    int historySize;
    std::vector<float> iaccHistory;
    std::vector<float> itdHistory;
    float itdVariance;
    float averageIACC;
    bool dataChanged = true;
    
    ShaderProgram shader;
    GLuint iaccVertexBuffer = 0;
    GLuint itdVertexBuffer = 0;
    
    juce::CriticalSection dataMutex;
};

} // namespace mojoGL
