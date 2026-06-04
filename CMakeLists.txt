cmake_minimum_required(VERSION 3.15)

project(NeuralVoiceFixer VERSION 0.1.0)

add_subdirectory(
    JUCE
)


add_compile_definitions(JUCE_VST3_CAN_REPLACE_VST2=0)

set(ONNXRUNTIME_ROOT
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/onnxruntime
)

set(ONNXRUNTIME_INCLUDE_DIR
    ${ONNXRUNTIME_ROOT}/include
)

set(ONNXRUNTIME_LIBRARY
    ${ONNXRUNTIME_ROOT}/lib/libonnxruntime.dylib
)

set(
    CMAKE_PREFIX_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libtorch"
)

set(PLUGIN_ASSETS
    ${CMAKE_SOURCE_DIR}/Source/Assets/voicefixer_generator.onnx
    ${CMAKE_SOURCE_DIR}/Source/Assets/voicefixer_vocoder.onnx
)

find_package(
    Torch REQUIRED
)


juce_add_plugin(NeuralVoiceFixer
    COMPANY_NAME "Feiyang"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSEL
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Feya
    PLUGIN_CODE Ns01
    FORMATS AU VST3
    PRODUCT_NAME "NeuralVoiceFixer"
)

target_sources(NeuralVoiceFixer PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/ONNX/OnnxModel.cpp
    Source/Helpers/Logger.cpp
    Source/DSP/STFT.cpp
    Source/DSP/MelFilterBank.cpp
    Source/DSP/VocoderPreprocessor.cpp
    Source/DSP/Resampler.cpp
    Source/VoiceFixerEngine.cpp
    Source/Assets/voicefixer_generator.onnx
    Source/Assets/voicefixer_vocoder.onnx
    Source/Helpers/resource.cpp
)

target_include_directories(
    NeuralVoiceFixer PRIVATE
    ${ONNXRUNTIME_INCLUDE_DIR}
)

target_compile_features(NeuralVoiceFixer PRIVATE cxx_std_17)

target_compile_options(
    NeuralVoiceFixer PRIVATE
    ${TORCH_CXX_FLAGS}
)

target_link_libraries(NeuralVoiceFixer PRIVATE

    ${ONNXRUNTIME_LIBRARY}
    ${TORCH_LIBRARIES}

    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra

    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
    juce::juce_recommended_lto_flags
)

foreach(FORMAT_TARGET
    NeuralVoiceFixer_AU
    NeuralVoiceFixer_VST3
)

    foreach(asset ${PLUGIN_ASSETS})

        add_custom_command(
            TARGET ${FORMAT_TARGET}
            POST_BUILD

            COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:${FORMAT_TARGET}>/../Resources"

            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${asset}
            "$<TARGET_FILE_DIR:${FORMAT_TARGET}>/../Resources/"
        )

    endforeach()

endforeach()

set_target_properties(
    NeuralVoiceFixer
    PROPERTIES

    BUILD_RPATH
    "${ONNXRUNTIME_ROOT}/lib"

    INSTALL_RPATH
    "@loader_path"
)