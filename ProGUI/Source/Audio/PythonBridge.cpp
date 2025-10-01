#include "PythonBridge.h"

PythonBridge::PythonBridge() {}
PythonBridge::~PythonBridge() {
    cancel();
}

void PythonBridge::extractStem(
    const juce::File& inputAudio,
    const juce::String& stemType,
    const juce::File& outputPath,
    std::function<void(StemResult)> callback,
    std::function<void(float)> progressCallback
) {
    shouldCancel = false;
    
    // Find Python executable
    auto pythonExe = findPythonExecutable();
    if (!pythonExe.existsAsFile()) {
        StemResult result;
        result.success = false;
        result.errorMessage = "Python executable not found. Please install Python 3.8+";
        callback(result);
        return;
    }
    
    // Get stem extractor script
    auto script = getStemExtractorScript();
    if (!script.existsAsFile()) {
        StemResult result;
        result.success = false;
        result.errorMessage = "Stem extractor script not found";
        callback(result);
        return;
    }
    
    // Prepare command arguments
    juce::StringArray args;
    args.add(pythonExe.getFullPathName());
    args.add(script.getFullPathName());
    args.add(inputAudio.getFullPathName());
    args.add(stemType);
    args.add(outputPath.getFullPathName());
    
    // Create and start process
    currentProcess = std::make_unique<juce::ChildProcess>();
    
    if (!currentProcess->start(args)) {
        StemResult result;
        result.success = false;
        result.errorMessage = "Failed to start Python process";
        callback(result);
        return;
    }
    
    // Monitor process in background thread
    juce::Thread::launch([this, callback, outputPath]() {
        if (!currentProcess) return;
        
        // Wait for process to complete
        int exitCode = currentProcess->waitForProcessToFinish(300000); // 5 min timeout
        
        if (shouldCancel) {
            StemResult result;
            result.success = false;
            result.errorMessage = "Extraction cancelled by user";
            callback(result);
            return;
        }
        
        // Read output
        juce::String output = currentProcess->readAllProcessOutput();
        
        // Parse JSON result
        auto jsonResult = juce::JSON::parse(output);
        
        StemResult result;
        if (jsonResult.isObject()) {
            auto obj = jsonResult.getDynamicObject();
            result.success = obj->getProperty("success");
            
            if (result.success) {
                result.outputFile = outputPath;
                result.sampleRate = obj->getProperty("samplerate");
            } else {
                result.errorMessage = obj->getProperty("error").toString();
            }
        } else {
            result.success = false;
            result.errorMessage = "Invalid response from Python process";
        }
        
        callback(result);
    });
}

bool PythonBridge::isPythonAvailable() const {
    auto pythonExe = findPythonExecutable();
    return pythonExe.existsAsFile();
}

bool PythonBridge::areDemucsModelsAvailable() const {
    // Check if demucs models are in expected location
    auto appDir = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                       .getParentDirectory();
    auto modelsDir = appDir.getChildFile("python_env/models/demucs");
    return modelsDir.exists();
}

void PythonBridge::cancel() {
    shouldCancel = true;
    if (currentProcess && currentProcess->isRunning()) {
        currentProcess->kill();
    }
}

juce::File PythonBridge::findPythonExecutable() const {
    // First check bundled Python
    auto appDir = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                       .getParentDirectory();
    auto bundledPython = appDir.getChildFile("python_env/bin/python3");
    
    if (bundledPython.existsAsFile())
        return bundledPython;
    
    // Check system Python
    juce::StringArray pythonPaths = {
        "/usr/local/bin/python3",
        "/opt/homebrew/bin/python3",
        "/usr/bin/python3",
        "/Library/Frameworks/Python.framework/Versions/3.11/bin/python3",
        "/Library/Frameworks/Python.framework/Versions/3.10/bin/python3"
    };
    
    for (const auto& path : pythonPaths) {
        juce::File pythonFile(path);
        if (pythonFile.existsAsFile())
            return pythonFile;
    }
    
    return {};
}

juce::File PythonBridge::getStemExtractorScript() const {
    auto appDir = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                       .getParentDirectory();
    
    // Check bundled scripts
    auto bundledScript = appDir.getChildFile("python_env/scripts/stem_extractor.py");
    if (bundledScript.existsAsFile())
        return bundledScript;
    
    // Check in Resources folder (macOS app bundle)
    auto resourceScript = appDir.getChildFile("../Resources/scripts/stem_extractor.py");
    if (resourceScript.existsAsFile())
        return resourceScript;
    
    return {};
}

juce::File PythonBridge::getTempOutputPath() const {
    return juce::File::createTempFile("stem_output");
}
