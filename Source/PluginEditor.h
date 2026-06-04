#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_dsp/juce_dsp.h>
#include "PluginProcessor.h"


class NeuralVoiceFixerAudioProcessorEditor
:
public juce::AudioProcessorEditor,
private juce::Button::Listener,
private juce::Timer
{
public:

    NeuralVoiceFixerAudioProcessorEditor(
        NeuralVoiceFixerAudioProcessor&
    );

    ~NeuralVoiceFixerAudioProcessorEditor()
        override;

    void paint(
        juce::Graphics&
    ) override;

    void resized()
        override;

    void mouseWheelMove(
        const juce::MouseEvent&,
        const juce::MouseWheelDetails&
    ) override;

private:

    void buttonClicked(
        juce::Button*
    ) override;

    void timerCallback()
        override;

    void drawTimeline(
        juce::Graphics&,
        juce::Rectangle<int>
    );

    void drawWaveform(
        juce::Graphics&,
        juce::Rectangle<int>,
        const juce::AudioBuffer<float>&,
        juce::Colour
    );

    void drawPlayhead(
        juce::Graphics&,
        juce::Rectangle<int>
    );

private:

    NeuralVoiceFixerAudioProcessor&
        processor;

    juce::TextButton
        recordButton;

    juce::TextButton
        restoreButton;

    juce::TextButton
        playButton;

    juce::TextButton
        zoomInButton;

    juce::TextButton
        zoomOutButton;
        

    double visibleStartTime =
        0.0;

    double visibleDuration =
        20.0;

    bool showExportReminder = false;

    double reminderStartTime = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        NeuralVoiceFixerAudioProcessorEditor
    )
};