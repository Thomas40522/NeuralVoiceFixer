#include "VoiceFixerEngine.h"
#include "Helpers/resource.h"

VoiceFixerEngine::VoiceFixerEngine()
{
    generator = OnnxModel();
    auto generatorFile = getResourceFile("voicefixer_generator.onnx");
    generator.loadModel(generatorFile.getFullPathName().toStdString());
    
    vocoder = OnnxModel();
    auto vocoderFile = getResourceFile("voicefixer_vocoder.onnx");
    vocoder.loadModel(vocoderFile.getFullPathName().toStdString());

    resampler = Resampler();
    stft = STFT();
    melFilterBank = MelFilterBank();
    vocoderPreprocessor = VocoderPreprocessor();
}

juce::AudioBuffer<float>
VoiceFixerEngine::
processChunk(
    const juce::AudioBuffer<float>& input
)
{
    auto waveform =
    torch::from_blob(
        const_cast<float*>(
            input.getReadPointer(0)
        ),
        {
            input.getNumSamples()
        },
        torch::kFloat32
    ).clone();

    waveform =
    resampler.process(
        waveform,
        sampleRate,
        modelSamplerate
    );

    waveform =
        waveform.unsqueeze(
            0
        );

    auto restored = process(waveform);

    restored = restored.squeeze();

    restored = restored.contiguous().to(torch::kCPU);

    restored = resampler.process(
        restored,
        modelSamplerate,
        sampleRate
    );

    const int outputSamples =
        static_cast<int>(
            restored.numel()
        );

    juce::AudioBuffer<float>
        output;

    output.setSize(
        1,
        outputSamples
    );

    std::memcpy(
        output.getWritePointer(0),
        restored.data_ptr<float>(),
        outputSamples
        * sizeof(float)
    );

    return output;
}


torch::Tensor
VoiceFixerEngine::process(
    const torch::Tensor& waveform
)
{

    auto mag =
        stft.process(
            waveform
        );

    auto mel =
        melFilterBank.process(
            mag
        );

    auto restoredMel =
        generator.run(
            mel
        );

    auto vocoderInput =
        vocoderPreprocessor.process(
            restoredMel
        );

    auto restoredAudio =
        vocoder.run(
            vocoderInput
        );

    return restoredAudio;
}

void VoiceFixerEngine::
setSampleRate(
    double newRate
)
{
    sampleRate = newRate;
}