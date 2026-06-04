#!/bin/bash

set -e

echo "Installing NeuralVoiceFixer dependencies..."

PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"

mkdir -p "$PROJECT_ROOT/ThirdParty"

# JUCE

if [ ! -d "$PROJECT_ROOT/JUCE" ]; then

```
echo "Downloading JUCE..."

git clone \
    --depth 1 \
    https://github.com/juce-framework/JUCE.git \
    "$PROJECT_ROOT/JUCE"
```

else

```
echo "JUCE already exists."
```

fi

# ONNX Runtime

if [ ! -d "$PROJECT_ROOT/ThirdParty/onnxruntime" ]; then

```
echo "Downloading ONNX Runtime..."

mkdir -p "$PROJECT_ROOT/ThirdParty"

curl -L \
    -o onnxruntime.tgz \
    https://github.com/microsoft/onnxruntime/releases/download/v1.22.0/onnxruntime-osx-arm64-1.22.0.tgz

tar -xzf onnxruntime.tgz

mv onnxruntime-osx-arm64-1.22.0 \
   "$PROJECT_ROOT/ThirdParty/onnxruntime"

rm onnxruntime.tgz
```

else

```
echo "ONNX Runtime already exists."
```

fi

# LibTorch

if [ ! -d "$PROJECT_ROOT/ThirdParty/libtorch" ]; then

```
echo "Downloading LibTorch..."

curl -L \
    -o libtorch.zip \
    https://download.pytorch.org/libtorch/cpu/libtorch-macos-arm64-2.7.1.zip

unzip libtorch.zip

mv libtorch \
   "$PROJECT_ROOT/ThirdParty/libtorch"

rm libtorch.zip
```

else

```
echo "LibTorch already exists."
```

fi

echo ""
echo "Dependencies installed successfully."
echo ""
echo "You may now run:"
echo ""
echo "mkdir -p build"
echo "cd build"
echo "cmake .."
echo "make -j$(sysctl -n hw.ncpu)"
