#include "VocoderPreprocessor.h"
#include <cmath>

VocoderPreprocessor::VocoderPreprocessor()
{
}

torch::Tensor
VocoderPreprocessor::getMelWeight()
{
    constexpr float a =
        18.8927416350036f;

    constexpr float b =
        0.0269863588184314f;

    auto x =
        torch::arange(
            1,
            129,
            torch::kFloat32
        );

    return a *
        torch::exp(
            b * x
        );
}

torch::Tensor
VocoderPreprocessor::ampToDb(
    const torch::Tensor& x
)
{
    auto minLevel =
        torch::exp(
            torch::tensor(
                MIN_LEVEL_DB
                / 20.0f
                * std::log(
                    10.0f
                )
            )
        );

    return
        20.0f *
        torch::log10(
            torch::maximum(
                x,
                minLevel
            )
        );
}

torch::Tensor
VocoderPreprocessor::normalize(
    const torch::Tensor& s
)
{
    return torch::clamp(
        (2.0f *
            MAX_ABS_VALUE)
        *
        (
            (s - MIN_DB)
            /
            (-MIN_DB)
        )
        -
        MAX_ABS_VALUE,

        -MAX_ABS_VALUE,
        MAX_ABS_VALUE
    );
}

torch::Tensor
VocoderPreprocessor::process(
    const torch::Tensor& inputMel
)
{
    auto mel =
        inputMel.clone();

    mel =
        torch::clamp(
            mel,
            -100.0f,
            5.0f
        );

    mel =
        torch::pow(
            10.0f,
            mel
        );

    auto melWeight =
        getMelWeight();

    mel =
        mel
        /
        melWeight.view(
            {
                1,
                1,
                1,
                128
            }
        );

    mel =
        normalize(
            ampToDb(
                torch::abs(
                    mel
                )
            )
            -
            20.0f
        );

    mel =
        mel
        .index(
            {
                torch::indexing::Slice(),
                0
            }
        )
        .transpose(
            1,
            2
        );

    const auto T =
        mel.size(
            2
        );

    const int64_t padTail =
        (T % 2)
        +
        4;

    auto padding =
        torch::full(
            {
                mel.size(0),
                128,
                padTail
            },
            -4.0f,
            mel.options()
        );

    mel =
        torch::cat(
            {
                mel,
                padding
            },
            2
        );

    return mel;
}