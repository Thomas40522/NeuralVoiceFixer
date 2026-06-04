#include "Resampler.h"

#include <juce_audio_basics/juce_audio_basics.h>

Resampler::Resampler()
{
}

torch::Tensor
Resampler::process(
    const torch::Tensor& input,
    double sourceRate,
    double targetRate
)
{


    auto cpuTensor =
        input
        .contiguous()
        .to(torch::kCPU);

    const float* data =
        cpuTensor.data_ptr<float>();

    const int inputSamples =
        cpuTensor.numel();

    std::vector<float>
        inputVector(
            data,
            data + inputSamples
        );

    const double ratio =
        targetRate /
        sourceRate;

    juce::LagrangeInterpolator
        interpolator;

    const int outputSamples =
        static_cast<int>(
            inputSamples
            * ratio
        );

    std::vector<float>
        outputVector(
            outputSamples
        );

    interpolator.process(
        1.0 / ratio,
        inputVector.data(),
        outputVector.data(),
        outputSamples
    );

    auto outputTensor =
        torch::from_blob(
            outputVector.data(),
            { outputSamples },
            torch::kFloat32
        ).clone();

    return outputTensor;
}