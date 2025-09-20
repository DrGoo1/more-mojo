#pragma once
#include <JuceHeader.h>

namespace mojoGL {

/**
 * Base class for OpenGL components
 * Handles context creation, attachment and basic rendering
 */
class GLComponent : public juce::Component, private juce::OpenGLRenderer {
public:
    GLComponent() {
        // Set up OpenGL context
        openGLContext.setRenderer(this);
        openGLContext.setComponentPaintingEnabled(false);
        openGLContext.setContinuousRepainting(true);
    }
    
    ~GLComponent() override {
        // Make sure to detach when destroyed
        detach();
    }
    
    void attach() {
        // Attach the OpenGL context to this component
        openGLContext.attachTo(*this);
    }
    
    void detach() {
        // Detach the OpenGL context
        openGLContext.detach();
    }
    
    // OpenGLRenderer interface
    void newOpenGLContextCreated() override {
        // Set up shaders, textures, etc.
        setupGL();
    }
    
    void renderOpenGL() override {
        // Clear the background
        juce::OpenGLHelpers::clear(juce::Colours::transparentBlack);
        
        // Call the subclass rendering method
        paintGL();
    }
    
    void openGLContextClosing() override {
        // Clean up shaders, textures, etc.
        cleanupGL();
    }
    
    // Methods to override in subclasses
    virtual void setupGL() {}
    virtual void paintGL() {}
    virtual void cleanupGL() {}
    
protected:
    juce::OpenGLContext openGLContext;
};

/**
 * Shader helper class
 * Simplifies creation and use of OpenGL shaders
 */
class ShaderProgram {
public:
    ShaderProgram() = default;
    ~ShaderProgram() {
        if (programID != 0) {
            glDeleteProgram(programID);
        }
    }
    
    bool compile(const juce::String& vertexShader, const juce::String& fragmentShader) {
        // Create program
        programID = glCreateProgram();
        
        // Compile vertex shader
        GLuint vertexShaderID = compileShader(GL_VERTEX_SHADER, vertexShader.toRawUTF8());
        if (vertexShaderID == 0) return false;
        
        // Compile fragment shader
        GLuint fragmentShaderID = compileShader(GL_FRAGMENT_SHADER, fragmentShader.toRawUTF8());
        if (fragmentShaderID == 0) {
            glDeleteShader(vertexShaderID);
            return false;
        }
        
        // Link program
        glAttachShader(programID, vertexShaderID);
        glAttachShader(programID, fragmentShaderID);
        glLinkProgram(programID);
        
        // Check link status
        GLint linkStatus;
        glGetProgramiv(programID, GL_LINK_STATUS, &linkStatus);
        
        // Clean up shaders
        glDetachShader(programID, vertexShaderID);
        glDetachShader(programID, fragmentShaderID);
        glDeleteShader(vertexShaderID);
        glDeleteShader(fragmentShaderID);
        
        if (linkStatus == GL_FALSE) {
            GLchar infoLog[1024];
            glGetProgramInfoLog(programID, sizeof(infoLog), nullptr, infoLog);
            errorMessage = "Program link error: " + juce::String(infoLog);
            glDeleteProgram(programID);
            programID = 0;
            return false;
        }
        
        return true;
    }
    
    void use() {
        if (programID != 0) {
            glUseProgram(programID);
        }
    }
    
    GLint getUniformLocation(const char* name) {
        return glGetUniformLocation(programID, name);
    }
    
    void setUniform(const char* name, float value) {
        glUniform1f(getUniformLocation(name), value);
    }
    
    void setUniform(const char* name, int value) {
        glUniform1i(getUniformLocation(name), value);
    }
    
    void setUniform(const char* name, float x, float y) {
        glUniform2f(getUniformLocation(name), x, y);
    }
    
    void setUniform(const char* name, float x, float y, float z) {
        glUniform3f(getUniformLocation(name), x, y, z);
    }
    
    void setUniform(const char* name, float x, float y, float z, float w) {
        glUniform4f(getUniformLocation(name), x, y, z, w);
    }
    
    juce::String getErrorMessage() const {
        return errorMessage;
    }
    
private:
    GLuint compileShader(GLenum type, const char* source) {
        GLuint shaderID = glCreateShader(type);
        
        glShaderSource(shaderID, 1, &source, nullptr);
        glCompileShader(shaderID);
        
        GLint compileStatus;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compileStatus);
        
        if (compileStatus == GL_FALSE) {
            GLchar infoLog[1024];
            glGetShaderInfoLog(shaderID, sizeof(infoLog), nullptr, infoLog);
            errorMessage = "Shader compile error: " + juce::String(infoLog);
            glDeleteShader(shaderID);
            return 0;
        }
        
        return shaderID;
    }
    
    GLuint programID = 0;
    juce::String errorMessage;
};

} // namespace mojoGL
