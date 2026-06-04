#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_dsp/juce_dsp.h>
#include "VoiceFixerEngine.h"
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

struct RestoreJob
{
    int64_t startSample;
    int64_t endSample;
    bool firstChunk;
};

class NeuralVoiceFixerAudioProcessor
    : public juce::AudioProcessor
{
public:

    NeuralVoiceFixerAudioProcessor();
    ~NeuralVoiceFixerAudioProcessor() override;

    void prepareToPlay(
        double sampleRate,
        int samplesPerBlock
    ) override;

    bool isHostPlaying() const;

    void releaseResources() override;

    void processBlock(
        juce::AudioBuffer<float>&,
        juce::MidiBuffer&
    ) override;

    bool hasEditor() const override;
    juce::AudioProcessorEditor* createEditor() override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;

    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void changeProgramName(
        int,
        const juce::String&
    ) override;

    void getStateInformation(
        juce::MemoryBlock&
    ) override;

    void setStateInformation(
        const void*,
        int
    ) override;

    void startRecording();
    void stopRecording();

    bool isRecording() const;

    void runRestoration();

    void enableRestoredPlayback();

    void disableRestoredPlayback();

    bool isRestoredPlaybackEnabled() const;

    const juce::AudioBuffer<float>&
    getOriginalTimeline() const;

    const juce::AudioBuffer<float>&
    getRestoredTimeline() const;

    void processTimeline();

    void processChunk(
        int startSample,
        int endSample,
        bool firstChunk
    );

    double getHostTimeSeconds() const;

    void restorationWorker();

    bool consumeRestorationFinishedFlag();

    void queueRestoreJob(
        int64_t startSample,
        int64_t endSample,
        bool firstChunk
    );

private:
    VoiceFixerEngine voiceFixerEngine;

    bool recording = false;

    bool playRestored = true;

    bool isPlaying = false;

    double currentSampleRate =
        44100.0;

    double hostTimeSeconds =
        0.0;
    
    int chunkLength = static_cast<int>(currentSampleRate * 10);
    int overlapLength = static_cast<int>(currentSampleRate);

    int64_t timelineEndSample = 0;

    juce::AudioBuffer<float>
        originalTimeline;

    juce::AudioBuffer<float>
        restoredTimeline;

    juce::CriticalSection
        timelineLock;

    std::thread processingThread;

    std::atomic<bool> processing = false;

    std::queue<RestoreJob> restoreQueue;

    std::mutex restoreQueueMutex;

    std::condition_variable restoreQueueCV;

    std::thread restoreWorker;

    std::atomic<bool> workerRunning { false };

    std::atomic<bool> recordingFinished { true };

    std::atomic<bool> setFirstChunk { false };

    int64_t nextChunkToProcess = 0;

    std::atomic<bool> restorationJustFinished { false };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        NeuralVoiceFixerAudioProcessor
    )
};