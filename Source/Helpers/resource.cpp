#include "resource.h"

juce::File getResourceFile(
    const juce::String& filename
)
{
    auto bundle =
        juce::File::getSpecialLocation(
            juce::File::currentExecutableFile
        );

    return bundle
        .getParentDirectory()
        .getParentDirectory()
        .getChildFile(
            "Resources"
        )
        .getChildFile(
            filename
        );
}