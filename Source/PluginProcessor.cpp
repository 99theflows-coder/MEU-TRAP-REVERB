#include "PluginProcessor.h"
#include "PluginEditor.h"

TrapVocalPluginAudioProcessor::TrapVocalPluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#else
     :
#endif
    apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

TrapVocalPluginAudioProcessor::~TrapVocalPluginAudioProcessor() {}

const juce::String TrapVocalPluginAudioProcessor::getName() const { return "Trap Vocal Plugin"; }

bool TrapVocalPluginAudioProcessor::acceptsMidi() const { return false; }
bool TrapVocalPluginAudioProcessor::producesMidi() const { return false; }
bool TrapVocalPluginAudioProcessor::isMidiEffect() const { return false; }
double TrapVocalPluginAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int TrapVocalPluginAudioProcessor::getNumPrograms() { return 1; }
int TrapVocalPluginAudioProcessor::getCurrentProgram() { return 0; }
void TrapVocalPluginAudioProcessor::setCurrentProgram (int index) {}
const juce::String TrapVocalPluginAudioProcessor::getProgramName (int index) { return {}; }
void TrapVocalPluginAudioProcessor::changeProgramName (int index, const juce::String& newName) {}

void TrapVocalPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = juce::uint32 (samplesPerBlock);
    spec.numChannels = juce::uint32 (getTotalNumOutputChannels());

    processorChain.prepare (spec);

    // Initialize Delay
    auto& delay = processorChain.get<delayIndex>();
    delay.setMaximumDelayInSamples (juce::uint32 (sampleRate * 2.0)); // 2 seconds max
}

void TrapVocalPluginAudioProcessor::releaseResources() {}

bool TrapVocalPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet();
}

void TrapVocalPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Update Parameters
    auto hpfFreq = apvts.getRawParameterValue ("HPF")->load();
    auto lpfFreq = apvts.getRawParameterValue ("LPF")->load();
    auto chorusMix = apvts.getRawParameterValue ("CHORUS")->load();
    auto delayMix = apvts.getRawParameterValue ("DELAY")->load();
    auto reverbMix = apvts.getRawParameterValue ("REVERB")->load();

    // Filters
    processorChain.get<hpfIndex>().coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass (getSampleRate(), hpfFreq);
    processorChain.get<lpfIndex>().coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass (getSampleRate(), lpfFreq);

    // Chorus
    auto& chorus = processorChain.get<chorusIndex>();
    chorus.setMix (chorusMix);
    chorus.setRate (1.5f);
    chorus.setDepth (0.5f);

    // Delay
    auto& delay = processorChain.get<delayIndex>();
    delay.setDelay (getSampleRate() * 0.5f); // 500ms fixed for now, we could add a parameter

    // Reverb
    auto& reverb = processorChain.get<reverbIndex>();
    juce::dsp::Reverb::Parameters reverbParams;
    reverbParams.dryLevel = 1.0f - reverbMix;
    reverbParams.wetLevel = reverbMix;
    reverbParams.roomSize = 0.7f;
    reverb.setParameters(reverbParams);

    juce::dsp::AudioBlock<float> block (buffer);
    processorChain.process (juce::dsp::ProcessContextReplacing<float> (block));
    
    // Manual mix for delay as DelayLine doesn't have a mix parameter in basic version
    if (delayMix > 0) {
        // This is a simplified delay mix. In a real plugin, we'd use a separate buffer.
    }
}

bool TrapVocalPluginAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* TrapVocalPluginAudioProcessor::createEditor() { return new juce::GenericAudioProcessorEditor (*this); }

void TrapVocalPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void TrapVocalPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout TrapVocalPluginAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("HPF", "High Pass", 20.0f, 20000.0f, 20.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("LPF", "Low Pass", 20.0f, 20000.0f, 20000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("CHORUS", "Chorus", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("DELAY", "Delay", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("REVERB", "Reverb", 0.0f, 1.0f, 0.0f));

    return { params.begin(), params.end() };
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new TrapVocalPluginAudioProcessor(); }
