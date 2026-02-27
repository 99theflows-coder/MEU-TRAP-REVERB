#pragma once
#include "PluginProcessor.h"

class TrapVocalPluginAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    TrapVocalPluginAudioProcessorEditor (TrapVocalPluginAudioProcessor& p) : AudioProcessorEditor (&p), processor (p) { setSize (400, 300); }
    void paint (juce::Graphics& g) override { g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId)); }
    void resized() override {}
private:
    TrapVocalPluginAudioProcessor& processor;
};
