#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "Helpers/logger.h"

NeuralVoiceFixerAudioProcessorEditor::
NeuralVoiceFixerAudioProcessorEditor(
    NeuralVoiceFixerAudioProcessor& p
)
:
AudioProcessorEditor(p),
processor(p)
{
    setSize(
        1200,
        700
    );

    recordButton.setButtonText(
        "Record"
    );

    restoreButton.setButtonText(
        "Restore"
    );

    playButton.setButtonText(
        "Play Original"
    );

    zoomInButton.setButtonText(
        "+"
    );

    zoomOutButton.setButtonText(
        "-"
    );

    addAndMakeVisible(
        recordButton
    );

    addAndMakeVisible(
        restoreButton
    );

    addAndMakeVisible(
        playButton
    );

    addAndMakeVisible(
        zoomInButton
    );

    addAndMakeVisible(
        zoomOutButton
    );

    recordButton.addListener(
        this
    );

    restoreButton.addListener(
        this
    );

    playButton.addListener(
        this
    );

    zoomInButton.addListener(
        this
    );

    zoomOutButton.addListener(
        this
    );

    startTimerHz(
        30
    );
}

NeuralVoiceFixerAudioProcessorEditor::
~NeuralVoiceFixerAudioProcessorEditor()
{
}

void NeuralVoiceFixerAudioProcessorEditor::
resized()
{
    auto area =
        getLocalBounds();

    auto top =
        area.removeFromTop(
            40
        );

    recordButton.setBounds(
        top.removeFromLeft(
            100
        )
    );

    restoreButton.setBounds(
        top.removeFromLeft(
            100
        )
    );

    playButton.setBounds(
        top.removeFromLeft(
            140
        )
    );

    zoomInButton.setBounds(
        top.removeFromLeft(
            50
        )
    );

    zoomOutButton.setBounds(
        top.removeFromLeft(
            50
        )
    );
}

void NeuralVoiceFixerAudioProcessorEditor::
paint(
    juce::Graphics& g
)
{
    g.fillAll(
        juce::Colours::black
    );

    auto area =
        getLocalBounds();

    area.removeFromTop(
        50
    );

    auto originalArea =
        area.removeFromTop(
            area.getHeight() / 2
        );

    auto restoredArea =
        area;

    drawTimeline(
        g,
        getLocalBounds()
            .removeFromTop(
                50
            )
    );

    drawWaveform(
        g,
        originalArea,
        processor.getOriginalTimeline(),
        juce::Colours::lime
    );

    drawWaveform(
        g,
        restoredArea,
        processor.getRestoredTimeline(),
        juce::Colours::cyan
    );

    drawPlayhead(
        g,
        getLocalBounds()
            .withTrimmedTop(
                50
            )
    );

    if (
        showExportReminder
    )
    {
        auto box =
            getLocalBounds()
                .withSizeKeepingCentre(
                    500,
                    80
                );

        g.setColour(
            juce::Colours::black
                .withAlpha(0.85f)
        );

        g.fillRoundedRectangle(
            box.toFloat(),
            10.0f
        );

        g.setColour(
            juce::Colours::orange
        );

        g.drawRoundedRectangle(
            box.toFloat(),
            10.0f,
            2.0f
        );

        g.setColour(
            juce::Colours::white
        );

        g.drawText(
            "Restoration Complete - Export audio before closing the project",
            box,
            juce::Justification::centred
        );
    }
}

void NeuralVoiceFixerAudioProcessorEditor::
drawTimeline(
    juce::Graphics& g,
    juce::Rectangle<int> area
)
{
    double pixelsPerSecond =
        area.getWidth()
        /
        visibleDuration;

    g.setColour(
        juce::Colours::darkgrey
    );

    for (
        int i = 0;
        i <= visibleDuration;
        ++i
    )
    {
        float x =
            i
            *
            pixelsPerSecond;

        g.drawVerticalLine(
            (int)x,
            0.0f,
            (float)getHeight()
        );
    }

    g.setColour(
        juce::Colours::white
    );

    g.drawText(
        juce::String(
            (int)visibleStartTime
        )
        +
        "s - "
        +
        juce::String(
            (int)(
                visibleStartTime
                +
                visibleDuration
            )
        )
        +
        "s",

        area.getWidth()
        - 120,

        0,

        120,

        20,

        juce::Justification::right
    );
}

void NeuralVoiceFixerAudioProcessorEditor::
drawWaveform(
    juce::Graphics& g,
    juce::Rectangle<int> area,
    const juce::AudioBuffer<float>& audio,
    juce::Colour colour
)
{
    if (
        audio.getNumSamples()
        == 0
    )
    {
        return;
    }

    auto* samples =
        audio.getReadPointer(
            0
        );

    const double sr =
        44100.0;

    const int startSample =
        (int)(
            visibleStartTime
            * sr
        );

    const int endSample =
        (int)(
            (visibleStartTime
             +
             visibleDuration)
            * sr
        );

    juce::Path path;

    const int width =
        area.getWidth();

    for (
        int x = 0;
        x < width;
        ++x
    )
    {
        int sampleIndex =
            juce::jmap(
                x,
                0,
                width,
                startSample,
                endSample
            );

        if (
            sampleIndex
            >=
            audio.getNumSamples()
        )
        {
            break;
        }

        float sample =
            samples[
                sampleIndex
            ];

        float y =
            juce::jmap(
                sample,
                -1.0f,
                1.0f,
                (float)area.getBottom(),
                (float)area.getY()
            );

        if (x == 0)
        {
            path.startNewSubPath(
                (float)x,
                y
            );
        }
        else
        {
            path.lineTo(
                (float)x,
                y
            );
        }
    }

    g.setColour(
        colour
    );

    g.strokePath(
        path,
        juce::PathStrokeType(
            1.0f
        )
    );
}

void NeuralVoiceFixerAudioProcessorEditor::
drawPlayhead(
    juce::Graphics& g,
    juce::Rectangle<int> area
)
{
    double hostTime =
        processor
        .getHostTimeSeconds();

    float x =
        (
            hostTime
            -
            visibleStartTime
        )
        *
        (
            area.getWidth()
            /
            visibleDuration
        );

    g.setColour(
        juce::Colours::red
    );

    g.drawLine(
        x,
        (float)area.getY(),
        x,
        (float)area.getBottom(),
        2.0f
    );
}

void NeuralVoiceFixerAudioProcessorEditor::
mouseWheelMove(
    const juce::MouseEvent&,
    const juce::MouseWheelDetails&
    wheel
)
{
    visibleStartTime -=
        wheel.deltaX
        *
        2.0;

    if (
        visibleStartTime
        <
        0.0
    )
    {
        visibleStartTime = 0.0;
    }

    repaint();
}

void NeuralVoiceFixerAudioProcessorEditor::
buttonClicked(
    juce::Button* button
)
{
    if (button == &recordButton)
    {
        if (
            processor.isRecording()
        )
        {
            processor.stopRecording();

            recordButton.setButtonText(
                "Record"
            );
        }
        else
        {
            processor.startRecording();

            recordButton.setButtonText(
                "Stop"
            );
        }
    }

    else if (
        button
        ==
        &restoreButton
    )
    {
        processor.runRestoration();
    }

    else if (
        button == &playButton
        &&
        !processor.isRestoredPlaybackEnabled()
    )
    {
        processor.enableRestoredPlayback();
        playButton.setButtonText(
            "Play Original"
        );
    }

    else if (
        button == &playButton
        &&
        processor.isRestoredPlaybackEnabled()
    )
    {
        processor.disableRestoredPlayback();
        playButton.setButtonText(
            "Play Restored"
        );
    }

    else if (
        button
        ==
        &zoomInButton
    )
    {
        visibleDuration *= 0.8;
    }

    else if (
        button
        ==
        &zoomOutButton
    )
    {
        visibleDuration *= 1.25;
    }
}

void NeuralVoiceFixerAudioProcessorEditor::
timerCallback()
{
    static double previousTime =
        0.0;

    static int stagnantFrames =
        0;

    const double currentTime =
        processor.getHostTimeSeconds();

    constexpr double epsilon =
        0.0001;

    if (
        std::abs(
            currentTime
            -
            previousTime
        )
        <
        epsilon
    )
    {
        ++stagnantFrames;
    }
    else
    {
        stagnantFrames = 0;
    }

    previousTime =
        currentTime;

    if (
        stagnantFrames == 10
    )
    {
        Logger::log(
            "Transport stopped"
        );

        if (
            processor.isRecording()
        )
        {
            processor.stopRecording();

            recordButton.setButtonText(
                "Record"
            );
        }

        if (
            !processor.isRestoredPlaybackEnabled()
        )
        {
            processor.enableRestoredPlayback();

            playButton.setButtonText(
                "Play Original"
            );
        }
    }

    if (
        processor
            .consumeRestorationFinishedFlag()
    )
    {
        showExportReminder = true;

        reminderStartTime =
            juce::Time::
                getMillisecondCounterHiRes();
    }

    if (
        showExportReminder
    )
    {
        double elapsed =
            (
                juce::Time::
                    getMillisecondCounterHiRes()
                -
                reminderStartTime
            )
            /
            1000.0;

        if (
            elapsed > 3.0
        )
        {
            showExportReminder = false;
        }
    }

    repaint();
}