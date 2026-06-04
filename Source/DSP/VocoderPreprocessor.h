#pragma once

#include <torch/torch.h>

class VocoderPreprocessor
{
public:

    VocoderPreprocessor();

    torch::Tensor process(
        const torch::Tensor& mel
    );

private:

    torch::Tensor getMelWeight();

    torch::Tensor ampToDb(
        const torch::Tensor& x
    );

    torch::Tensor normalize(
        const torch::Tensor& s
    );

private:

    static constexpr float MIN_DB =
        -115.0f;

    static constexpr float MIN_LEVEL_DB =
        -100.0f;

    static constexpr float MAX_ABS_VALUE =
        4.0f;
};