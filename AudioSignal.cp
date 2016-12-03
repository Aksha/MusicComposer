#include "AudioSignal.h"
#include <iterator>
#include <iostream>
#include <algorithm>
#include <cmath>

AudioSignal::AudioSignal(const int sampleRate, const short bitsPerSample) :
    sampleRate(sampleRate), bitsPerSample(bitsPerSample) {
    // Nothing else to be done.
}

AudioSignal::AudioSignal(RIFFReader& inputWaveFile) {
    sampleRate    = inputWaveFile.getSampleRate();
    bitsPerSample = inputWaveFile.getBitsPerSample();
    Sample s;
    while ((inputWaveFile >> s)) {
        wave.push_back(s);
    }
}

AudioSignal::AudioSignal(const AudioSignal& src) :
    sampleRate(src.sampleRate), bitsPerSample(src.bitsPerSample),
    wave(src.wave) {
    // Nothing else to be done.
}

AudioSignal::AudioSignal(AudioSignal&& src) :
    sampleRate(std::move(src.sampleRate)),
    bitsPerSample(std::move(src.bitsPerSample)),
    wave(std::move(src.wave)) {
    // Nothing else to be done.
}

AudioSignal::AudioSignal(const int freq, const int lenInMills,
                         const int amplitude, const int sampleRate,
                         const short bitsPerSample, const bool decay)
    : sampleRate(sampleRate), bitsPerSample(bitsPerSample) {
    const int SampleCount  = sampleRate * lenInMills / 1000;
    const int samplesPerHz = sampleRate / freq;
    const double toRad     = (360.0 / samplesPerHz) * (M_PI / 180.0);
    for(int i = 0; (i < SampleCount); i++) {
        double angle = (i % samplesPerHz) * toRad;
        int data = amplitude * std::sin(angle) *
            (decay ? exp((double) -i / SampleCount / 0.5) : 1);
        wave.push_back(data);
    }
}

AudioSignal&
AudioSignal::operator=(const AudioSignal& src) {
    sampleRate    = src.sampleRate;
    bitsPerSample = src.bitsPerSample;
    wave          = src.wave;
    return *this;
}
    
AudioSignal
AudioSignal::operator+(const AudioSignal& other) const {
    // Select the longer of the two signals as sig1 and shorter one as sig2
    const bool isThisLonger = (wave.size() > other.wave.size());
    const AudioSignal &sig1 = (isThisLonger ? *this : other);
    const AudioSignal &sig2 = (isThisLonger ? other : *this);
    AudioSignal sum(sig1);
    std::transform(sig2.wave.begin(), sig2.wave.end(), sum.wave.begin(),
                   sum.wave.begin(), std::plus<Sample>());
    return sum;
}

AudioSignal&
AudioSignal::operator&=(const AudioSignal& other) {
    wave.insert(wave.end(), other.wave.begin(), other.wave.end());
    return *this;
}

AudioSignal&
AudioSignal::operator*=(double factor) {
    std::transform(wave.begin(), wave.end(), wave.begin(),
                   [factor](Sample s){ return s * factor; });
    return *this;
}

AudioSignal
AudioSignal::operator>>(long timeInMillis) const {
    AudioSignal as(1, timeInMillis, 0, sampleRate, bitsPerSample, false);
    as &= *this;
    return as;
}

AudioSignal
AudioSignal::operator<<(long timeInMillis) const {
    AudioSignal as;
    const size_t numToRemove = timeInMillis * sampleRate / 1000;
    if (numToRemove < wave.size()) {
        as.wave.insert(as.wave.end(), wave.begin() + numToRemove,
                       wave.end());
    }
    return as;
}

void
AudioSignal::write(const std::string& fileName) const throw (std::exception) {
    RIFFWriter outFile(fileName, sampleRate, bitsPerSample);
    std::for_each(wave.begin(), wave.end(),
                  [&outFile](const Sample s) { outFile << s; });
}

long
AudioSignal::duration() const {
    return wave.size() * 1000 / sampleRate;
}

void
AudioSignal::changeDurationTo(const int timeInMillis) {
    // Grow the signal to the desired duration if needed.
    AudioSignal orig(*this); // Original audio signal.
    while (duration() < timeInMillis) {
        *this &= orig; // Grow wave.
    }
    // Shrink wave to appropriate duration.
    wave.resize(timeInMillis * sampleRate / 1000);
}
