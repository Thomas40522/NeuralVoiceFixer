#pragma once

#include <torch/torch.h>

class MelFilterBank
{
public:

    MelFilterBank();

    torch::Tensor process(
        const torch::Tensor& magnitude
    );

private:

    torch::Tensor filterbank;
};