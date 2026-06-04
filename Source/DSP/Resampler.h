#pragma once

#include <torch/torch.h>

class Resampler
{
public:

    Resampler();

    torch::Tensor process(
        const torch::Tensor& input,
        double sourceRate,
        double targetRate
    );
};