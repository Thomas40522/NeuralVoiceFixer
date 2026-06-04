#include "MelFilterBank.h"

#include "../Assets/mel_filterbank_data.h"
#include "../Helpers/logger.h"

MelFilterBank::MelFilterBank()
{
    filterbank =
        torch::from_blob(
            (void*)MEL_FILTERBANK,
            {
                MEL_ROWS,
                MEL_COLS
            },
            torch::kFloat32
        ).clone();

    Logger::log(
        "Mel filterbank loaded"
    );
}

torch::Tensor MelFilterBank::process(
    const torch::Tensor& magnitude
)
{

    auto mel =
        torch::matmul(
            magnitude.transpose(
                -1,
                -2
            ),
            filterbank
        );

    mel =
        mel.transpose(
            -1,
            -2
        );

    mel =
        mel.permute(
            {
                0,
                1,
                3,
                2
            }
        );

    return mel;
}