#include "STFT.h"
#include "../Helpers/logger.h"

STFT::STFT()
{
    window =
        torch::hann_window(
            2048,
            torch::kFloat32
        );

    Logger::log(
        "STFT initialized"
    );
}

torch::Tensor STFT::process(
    const torch::Tensor& waveform
)
{

    Logger::log(
        "Running STFT..."
    );

    auto stft = torch::stft(
        waveform,
        2048,           // n_fft
        441,            // hop_length
        2048,           // win_length
        window,
        true,           // center
        "reflect",      // pad_mode
        false,          // normalized
        std::nullopt,   // onesided
        true            // return_complex
    );

    Logger::log(
        "Computing magnitude..."
    );

    auto mag = torch::abs(stft);

    Logger::log(
        "Adding channel dimension..."
    );

    mag = mag.unsqueeze(1);

    return mag;
    
}

void STFT::logShape(
    const torch::Tensor& tensor
)
{
    std::string shape =
        "Shape: ";

    for (
        auto d :
        tensor.sizes()
    )
    {
        shape +=
            std::to_string(d)
            + " ";
    }

    Logger::log(shape);
}