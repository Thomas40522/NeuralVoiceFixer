# NeuralVoiceFixer

NeuralVoiceFixer is a JUCE-based audio restoration plugin powered by the VoiceFixer neural network.

The plugin records audio directly from the DAW timeline, restores it using a neural network pipeline, and allows A/B comparison between the original and restored audio.

## Features

* Timeline-based audio capture
* VoiceFixer neural restoration
* Background chunk processing
* Seamless chunk crossfading
* Original / Restored playback switching
* AU and VST3 support
* Native Apple Silicon support

## Current Status

NeuralVoiceFixer is currently an experimental project and under active development.

Implemented:

* Recording from the DAW timeline
* Automatic chunk processing
* Background restoration worker
* Playback of restored audio
* Timeline waveform visualization

Planned:

* Export restored audio
* Persistent project storage
* Windows support
* GPU acceleration

## Building

### macOS (Apple Silicon)

Install dependencies:

```bash
./install.sh
```

Configure and build:

```bash
mkdir build
cd build

cmake ..
make -j$(sysctl -n hw.ncpu)
```

Build artifacts are generated in:

```text
build/NeuralVoiceFixer_artefacts/
```

## Dependencies

* JUCE
* ONNX Runtime
* LibTorch
* VoiceFixer

## Usage

1. Insert NeuralVoiceFixer on a track.
2. Press Record.
3. Play the DAW timeline.
4. Stop playback.
5. Wait for restoration to complete.
6. Switch between Original and Restored playback.

⚠ Audio is currently stored in memory only. Export restored audio before closing the DAW project.

## License

This repository contains the plugin implementation only.

VoiceFixer and any associated model files remain subject to their respective licenses.
