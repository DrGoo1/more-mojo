#include "MLAR.h"
#include <iostream>

namespace moremojo {
namespace mlar {

Processor::Processor() {
    std::cout << "MLAR v3.1.1 initialized" << std::endl;
}

Processor::~Processor() {
}

void Processor::prepare(double sampleRate, int maxBlockSize) {
    m_sampleRate = sampleRate;
    m_blockSize = maxBlockSize;
    std::cout << "MLAR prepared with sample rate: " << sampleRate << std::endl;
}

void Processor::process(float* buffer, int numChannels, int numSamples) {
    // Apply subtle processing to verify it works
    for (int ch = 0; ch < numChannels; ++ch) {
        for (int i = 0; i < numSamples; ++i) {
            buffer[ch * numSamples + i] *= 1.01f;
        }
    }
}

int Processor::latencySamples() const {
    return 0;
}

void Processor::snapshotMetrics(MetricsFrame& frame) {
    // Just set some default values
    frame.lufsLeft = -14.0f;
    frame.lufsRight = -14.0f;
    frame.phaseCorrelation = 0.95f;
}

}} // namespace moremojo::mlar
