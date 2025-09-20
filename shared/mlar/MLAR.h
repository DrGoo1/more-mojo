#pragma once
#include <string>

namespace moremojo {
namespace mlar {

// Forward declarations
class Processor;

// Metrics frame - holds all analysis metrics
struct MetricsFrame {
    float lufsLeft = 0.0f;
    float lufsRight = 0.0f;
    float targetLUFS = -14.0f;
    float phaseCorrelation = 1.0f;
    float transientActivity = 0.0f;
    float gdsScore = 0.0f;
    float alignmentScore = 0.0f;
    float limitingAmount = 0.0f;
};

// MLAR Processor - main processing class
class Processor {
public:
    Processor();
    ~Processor();
    
    void prepare(double sampleRate, int maxBlockSize);
    void process(float* buffer, int numChannels, int numSamples);
    int latencySamples() const;
    void snapshotMetrics(MetricsFrame& frame);
    
    static const char* getVersion() { return "3.1.1"; }
    
private:
    double m_sampleRate = 44100.0;
    int m_blockSize = 512;
};

}} // namespace moremojo::mlar
