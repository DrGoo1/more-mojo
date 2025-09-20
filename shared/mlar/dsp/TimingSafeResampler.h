#pragma once
#include "../MLAR.h"

namespace moremojo {
namespace mlar {

class TimingSafeResampler {
public:
    TimingSafeResampler() = default;
    ~TimingSafeResampler() = default;
    
    void prepare(double sampleRate, int maxBlockSize);
    void process(float* buffer, int numChannels, int numSamples);
    int latencySamples() const { return 0; }
};

}} // namespace moremojo::mlar
