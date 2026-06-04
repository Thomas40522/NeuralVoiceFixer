#pragma once

#include "DSP/Resampler.h"
#include "DSP/STFT.h"
#include "DSP/MelFilterBank.h"
#include "DSP/VocoderPreprocessor.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_dsp/juce_dsp.h>

#include "ONNX/OnnxModel.h"

class VoiceFixerEngine
{
public:

    VoiceFixerEngine();

    torch::Tensor process(
        const torch::Tensor& waveform
    );

    juce::AudioBuffer<float>
    processChunk(
        const juce::AudioBuffer<float>&
    );

    void setSampleRate(double sampleRate);

private:

    Resampler resampler;

    STFT stft;

    MelFilterBank melFilterBank;

    VocoderPreprocessor
        vocoderPreprocessor;

    OnnxModel generator;

    OnnxModel vocoder;

    double sampleRate = 44100.0;

    const double modelSamplerate = 44100.0;
};