#include "OnnxModel.h"
#include "../Helpers/logger.h"

#include <iostream>

OnnxModel::OnnxModel()
:
env(
    ORT_LOGGING_LEVEL_WARNING,
    "NeuralVoiceFixer"
)
{
}

bool OnnxModel::loadModel(
    const std::string& modelPath
)
{
    try
    {
        Logger::log(
            "Loading model..."
        );
        Ort::SessionOptions sessionOptions;

        sessionOptions.SetGraphOptimizationLevel(
            GraphOptimizationLevel::ORT_ENABLE_ALL
        );

        session =
            std::make_unique<Ort::Session>(
                env,
                modelPath.c_str(),
                sessionOptions
            );

        Logger::log(
            "Model loaded successfully"
        );

        Ort::AllocatorWithDefaultOptions allocator;

        size_t numInputs =
            session->GetInputCount();

        size_t numOutputs =
            session->GetOutputCount();

        Logger::log(
            "Inputs: " + std::to_string(numInputs)
        );

        Logger::log(
            "Outputs: " + std::to_string(numOutputs)
        );

        for (
            size_t i = 0;
            i < numInputs;
            ++i
        )
        {
            auto name =
                session->GetInputNameAllocated(
                    i,
                    allocator
                );

            if (i == 0) {
                inputName = name.get();
            }

            Logger::log(
                "Input " + std::to_string(i) + ": " + name.get()
            );

            auto typeInfo =
                session->GetInputTypeInfo(i);

            auto tensorInfo =
                typeInfo
                .GetTensorTypeAndShapeInfo();

            auto shape =
                tensorInfo.GetShape();

            modelInputShape = tensorInfo.GetShape();

            std::string shapeString = "Shape: ";

            for (auto d : shape)
            {
                shapeString +=
                    std::to_string(d)
                    + " ";
            }

            Logger::log(shapeString);
        }

        for (
            size_t i = 0;
            i < numOutputs;
            ++i
        )
        {
            auto name =
                session->GetOutputNameAllocated(
                    i,
                    allocator
                );

            if (i == 0) {
                outputName = name.get();
            }

            Logger::log(
                "Output " + std::to_string(i) + ": " + name.get()
            );
        }


        return true;
    }
    catch (
        const Ort::Exception& e
    )
    {
        Logger::log(
            std::string("ONNX Error: ")
            + e.what()
        );

        return false;
    }
}


torch::Tensor
OnnxModel::run(
    const torch::Tensor& input
)
{
    if (!session)
    {
        Logger::log(
            "Session not loaded"
        );

        return {};
    }

    try
    {
        auto inputCpu =
            input.contiguous();

        auto shape =
            inputCpu.sizes();

        std::vector<int64_t>
            dims(
                shape.begin(),
                shape.end()
            );
        
        Ort::MemoryInfo memoryInfo =
            Ort::MemoryInfo::CreateCpu(
                OrtArenaAllocator,
                OrtMemTypeDefault
            );

        auto inputTensor =
            Ort::Value::CreateTensor<float>(
                memoryInfo,
                inputCpu.data_ptr<float>(),
                inputCpu.numel(),
                dims.data(),
                dims.size()
            );


        const char* inputNames[]
        {
            inputName.c_str()
        };

        const char* outputNames[]
        {
            outputName.c_str()
        };

        auto outputs =
            session->Run(
                Ort::RunOptions{nullptr},
                inputNames,
                &inputTensor,
                1,
                outputNames,
                1
            );
        
        auto& output =
            outputs.front();

        auto outputInfo =
            output
            .GetTensorTypeAndShapeInfo();

        auto outputShape =
            outputInfo
            .GetShape();

        auto outputSize =
            outputInfo
            .GetElementCount();

        float* outputData =
            output
            .GetTensorMutableData<float>();

        auto tensor =
            torch::from_blob(
                outputData,
                outputShape,
                torch::kFloat32
            ).clone();

        return tensor;
    }
    catch (
        const Ort::Exception& e
    )
    {
        Logger::log(
            std::string(
                "Inference Error: "
            )
            + e.what()
        );

        return {};
    }
}