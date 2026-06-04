#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Helpers/logger.h"
#include "DSP/STFT.h"
#include "DSP/MelFilterBank.h"
#include "DSP/VocoderPreprocessor.h"

NeuralVoiceFixerAudioProcessor::
NeuralVoiceFixerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(
        BusesProperties()
            .withInput(
                "Input",
                juce::AudioChannelSet::stereo(),
                true
            )
            .withOutput(
                "Output",
                juce::AudioChannelSet::stereo(),
                true
            )
    )
#endif
{
    Logger::log("\nPlugin Loaded");

}

NeuralVoiceFixerAudioProcessor::
~NeuralVoiceFixerAudioProcessor()
{
    recordingFinished = true;

    restoreQueueCV.notify_all();

    if (
        restoreWorker.joinable()
    )
    {
        restoreWorker.join();
    }
}

void NeuralVoiceFixerAudioProcessor::
prepareToPlay(
    double sampleRate,
    int
)
{
    currentSampleRate =
        sampleRate;


    Logger::log(
        "DAW Sample Rate = "
        + std::to_string(sampleRate)
    );

    chunkLength = static_cast<int>(currentSampleRate * 10);
    overlapLength = static_cast<int>(currentSampleRate);

    voiceFixerEngine.setSampleRate(sampleRate);

    const int maxSamples =
        static_cast<int>(
            sampleRate
            *
            60.0
            *
            30.0
        );

    originalTimeline.setSize(
        1,
        maxSamples
    );

    restoredTimeline.setSize(
        1,
        maxSamples
    );

    originalTimeline.clear();

    restoredTimeline.clear();
}

void NeuralVoiceFixerAudioProcessor::
startRecording()
{
    Logger::log(
        "startRecording"
    );

    recording = true;

    

    recordingFinished = false;

    setFirstChunk = true;

    if (!workerRunning) {
        if (
            restoreWorker.joinable()
        )
        {
            Logger::log(
                "Joining previous worker"
            );

            restoreWorker.join();
        }
        workerRunning = true;

        restoreWorker =
            std::thread(
                [this]
                {
                    restorationWorker();
                }
            );
    }

}

void NeuralVoiceFixerAudioProcessor::
stopRecording()
{
    recording = false;
    if (
        timelineEndSample
        >
        nextChunkToProcess
    )
    {
        queueRestoreJob(
            nextChunkToProcess,
            timelineEndSample,
            false
        );
    }

    recordingFinished = true;

    restoreQueueCV.notify_one();
}

bool NeuralVoiceFixerAudioProcessor::
isRecording() const
{
    return recording;
}

bool NeuralVoiceFixerAudioProcessor::
consumeRestorationFinishedFlag()
{
    return
        restorationJustFinished
            .exchange(false);
}

void NeuralVoiceFixerAudioProcessor::
enableRestoredPlayback()
{
    playRestored = true;
}

void NeuralVoiceFixerAudioProcessor::
disableRestoredPlayback()
{
    playRestored = false;
}

bool NeuralVoiceFixerAudioProcessor::isRestoredPlaybackEnabled() const
{
    return playRestored;
}

const juce::AudioBuffer<float>&
NeuralVoiceFixerAudioProcessor::
getOriginalTimeline() const
{
    return originalTimeline;
}

const juce::AudioBuffer<float>&
NeuralVoiceFixerAudioProcessor::
getRestoredTimeline() const
{
    return restoredTimeline;
}

double NeuralVoiceFixerAudioProcessor::
getHostTimeSeconds() const
{
    return hostTimeSeconds;
}

void NeuralVoiceFixerAudioProcessor::
runRestoration()
{

    if (processing)
        return;

    processing = true;

    processingThread =
        std::thread(
            [this]
            {
                processTimeline();
            }
        );

    processingThread.detach();

}

void NeuralVoiceFixerAudioProcessor::
processTimeline()
{
    Logger::log(
        "Starting restoration..."
    );

    const int totalSamples = timelineEndSample;

    bool firstChunk = true;
    

    for (
        int start = 0;
        start < totalSamples;
        start += (
            chunkLength
            -
            overlapLength
        )
    )
    {
        int end =
            std::min(
                start + chunkLength,
                totalSamples
            );
        Logger::log("chunkLength = " + std::to_string(chunkLength));
        Logger::log("overlapLength = " + std::to_string(overlapLength));

        Logger::log(
            "Processing chunk "
            +
            std::to_string(start)
            +
            " -> "
            +
            std::to_string(end)
        );

        processChunk(
            start,
            end,
            firstChunk
        );

        firstChunk = false;
    }

    Logger::log(
        "Restoration Complete"
    );

    processing = false;
}


void NeuralVoiceFixerAudioProcessor::
processChunk(
    int startSample,
    int endSample,
    bool firstChunk
)
{
    const int length =
        endSample
        -
        startSample;


    juce::AudioBuffer<float>
        chunk;

    chunk.setSize(
        1,
        length
    );

    chunk.copyFrom(
        0,
        0,
        originalTimeline,
        0,
        startSample,
        length
    );

    auto restored =
        voiceFixerEngine
        .processChunk(
            chunk
        );


    if (firstChunk)
    {
        restoredTimeline.copyFrom(
            0,
            startSample,
            restored,
            0,
            0,
            restored.getNumSamples()
        );

        return;
    }

    const int overlapStart =
    startSample;

    Logger::log("overlapSamples = " + std::to_string(overlapLength));

    for (
        int i = 0;
        i < overlapLength;
        ++i
    )
    {
        float t =
            (float)i
            /
            overlapLength;

        float fadeOut =
            std::cos(
                t
                *
                juce::MathConstants<float>::halfPi
            );

        float fadeIn =
            std::sin(
                t
                *
                juce::MathConstants<float>::halfPi
            );

        float existing =
            restoredTimeline.getSample(
                0,
                overlapStart + i
            );

        float incoming =
            restored.getSample(
                0,
                i
            );

        restoredTimeline.setSample(
            0,
            overlapStart + i,
            existing * fadeOut
            +
            incoming * fadeIn
        );
    }

    const int remainderStart = overlapLength;

    const int remainderLength =
        restored.getNumSamples()
        -
        overlapLength;

    restoredTimeline.copyFrom(
        0,
        startSample
        +
        overlapLength,

        restored,
        0,
        overlapLength,

        remainderLength
    );

}

void NeuralVoiceFixerAudioProcessor::
restorationWorker()
{
    while (true)
    {
        RestoreJob job;

        {
            std::unique_lock<std::mutex>
                lock(
                    restoreQueueMutex
                );

            restoreQueueCV.wait(
                lock,
                [this]
                {
                    return
                        !restoreQueue.empty()
                        ||
                        recordingFinished.load();
                }
            );

            if (
                restoreQueue.empty()
                &&
                recordingFinished.load()
            )
            {
                break;
            }

            job =
                restoreQueue.front();

            restoreQueue.pop();
        }

        Logger::log(
            "Processing Chunk "
            +
            std::to_string(
                job.startSample
            )
        );

        processChunk(
            job.startSample,
            job.endSample,
            job.firstChunk
        );
    }

    Logger::log(
        "Worker Finished"
    );

    workerRunning = false;

    restorationJustFinished = true;
}

void NeuralVoiceFixerAudioProcessor::
queueRestoreJob(
    int64_t startSample,
    int64_t endSample,
    bool firstChunk
)
{
    {
        std::lock_guard<std::mutex>
            lock(
                restoreQueueMutex
            );

        restoreQueue.push(
        {
            startSample,
            endSample,
            firstChunk
        });
    }

    restoreQueueCV.notify_one();
}


const juce::String
NeuralVoiceFixerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool
NeuralVoiceFixerAudioProcessor::acceptsMidi() const
{
    return false;
}

bool
NeuralVoiceFixerAudioProcessor::producesMidi() const
{
    return false;
}

bool
NeuralVoiceFixerAudioProcessor::isMidiEffect() const
{
    return false;
}

double
NeuralVoiceFixerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int
NeuralVoiceFixerAudioProcessor::getNumPrograms()
{
    return 1;
}

int
NeuralVoiceFixerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void
NeuralVoiceFixerAudioProcessor::setCurrentProgram(int)
{
}

const juce::String
NeuralVoiceFixerAudioProcessor::getProgramName(int)
{
    return {};
}

void
NeuralVoiceFixerAudioProcessor::changeProgramName(
    int,
    const juce::String&
)
{
}

void
NeuralVoiceFixerAudioProcessor::releaseResources()
{
}

void NeuralVoiceFixerAudioProcessor::
processBlock(
    juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer&
)
{
    if (
        auto* playHead =
        getPlayHead()
    )
    {
        if (
            auto position =
            playHead->getPosition()
        )
        {
            if (
                auto time =
                position->getTimeInSeconds()
            )
            {
                hostTimeSeconds =
                    *time;
            }
        }
    }

    if (recording)
    {


        const juce::ScopedLock lock(
            timelineLock
        );

        int64_t timelineStart =
            static_cast<int64_t>(
                hostTimeSeconds
                *
                currentSampleRate
            );
        

        if (timelineStart < 0) {
            Logger::log(
                "Negative timeline start adjusted to 0"
            );
            timelineStart = 0;
        }

        if (setFirstChunk == true) {
            Logger::log(
                "nextChunk to process set: " + std::to_string(timelineStart)
            );
            nextChunkToProcess = timelineStart;
            setFirstChunk = false;
        }

        originalTimeline.copyFrom(
            0,
            (int)timelineStart,
            buffer,
            0,
            0,
            buffer.getNumSamples()
        );

        const int64_t writeEnd =
            timelineStart
            +
            buffer.getNumSamples();

        timelineEndSample =
            std::max(
                timelineEndSample,
                writeEnd
            );
        
        while (
            timelineEndSample
            >=
            nextChunkToProcess
            +
            chunkLength
        )
        {
            queueRestoreJob(
                nextChunkToProcess,

                nextChunkToProcess
                +
                chunkLength,

                nextChunkToProcess == 0
            );

            Logger::log(
                "Queued chunk "
                +
                std::to_string(
                    nextChunkToProcess
                )
            );

            nextChunkToProcess +=
                (
                    chunkLength
                    -
                    overlapLength
                );
        }

    }
    if (playRestored && !recording)
    {
        const juce::ScopedLock lock(
            timelineLock
        );

        const int64_t timelineStart =
            static_cast<int64_t>(
                hostTimeSeconds
                *
                currentSampleRate
            );

        static int64_t previousTimelineStart = -1;

        if (timelineStart == previousTimelineStart)
        {
            buffer.clear();
            return;
        }

        previousTimelineStart = timelineStart;

        buffer.copyFrom(
            0,
            0,
            restoredTimeline,
            0,
            (int)timelineStart,
            buffer.getNumSamples()
        );

        if (
            buffer.getNumChannels()
            > 1
        )
        {
            buffer.copyFrom(
                1,
                0,
                restoredTimeline,
                0,
                (int)timelineStart,
                buffer.getNumSamples()
            );
        }


    }
}

bool
NeuralVoiceFixerAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor*
NeuralVoiceFixerAudioProcessor::createEditor()
{
    return new NeuralVoiceFixerAudioProcessorEditor(*this);
}

void
NeuralVoiceFixerAudioProcessor::getStateInformation(
    juce::MemoryBlock&
)
{
}

void
NeuralVoiceFixerAudioProcessor::setStateInformation(
    const void*,
    int
)
{
}


juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NeuralVoiceFixerAudioProcessor();
}