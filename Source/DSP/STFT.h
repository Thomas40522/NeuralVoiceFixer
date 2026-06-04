#pragma once

#include <torch/torch.h>

class STFT
{
public:

    STFT();

    torch::Tensor process(
        const torch::Tensor& waveform
    );

    static void logShape(
        const torch::Tensor& tensor
    );

private:

    torch::Tensor window;
};