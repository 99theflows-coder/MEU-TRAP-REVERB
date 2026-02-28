cmake_minimum_required(VERSION 3.15)

project(TrapVocalPlugin VERSION 0.0.1)

# Corrigido para encontrar o JUCE no GitHub Actions
add_subdirectory(JUCE)

juce_add_plugin(TrapVocalPlugin
    COMPANY_NAME "Manus"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Manu
    PLUGIN_CODE Trap
    FORMATS VST3
    PRODUCT_NAME "TrapVocalPlugin")

target_sources(TrapVocalPlugin
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h)

target_compile_definitions(TrapVocalPlugin
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_AUDIO_PROCESSOR_NO_GUI=1
        JUCE_GUI_BASICS_INCLUDE_XHEADERS=0)

target_link_libraries(TrapVocalPlugin
    PRIVATE
        juce::juce_audio_processors
        juce::juce_audio_basics
        juce::juce_dsp
        juce::juce_core
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
