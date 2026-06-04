#pragma once

#include <onnxruntime_cxx_api.h>

#include <memory>
#include <string>
#include <torch/torch.h>

class OnnxModel
{
public:

    OnnxModel();

    bool loadModel(
        const std::string& modelPath
    );

    torch::Tensor run(
        const torch::Tensor& input
    );

private:

    Ort::Env env;

    std::unique_ptr<Ort::Session> session;

    std::string inputName;
    std::string outputName;

    std::vector<int64_t> modelInputShape;
};