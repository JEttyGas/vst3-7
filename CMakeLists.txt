cmake_minimum_required(VERSION 3.22)
# Полная версия плагина 0.2.2.8 показывается в интерфейсе (Source/Version.h).
# В метаданные JUCE/VST3 передаются три числа 0.2.2.
project(BeatMorph VERSION 0.2.2)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    ON)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(BeatMorph
    COMPANY_NAME "MyCompany"
    PLUGIN_MANUFACTURER_CODE Mycm
    PLUGIN_CODE Bmph
    FORMATS VST3 Standalone
    PRODUCT_NAME "BeatMorph"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE)

target_sources(BeatMorph PRIVATE
    Source/Patterns.h
    Source/Presets.h
    Source/PresetsV2.h
    Source/PresetsV3.h
    Source/Spectral.h
    Source/Delay.h
    Source/Dist.h
    Source/Look.h
    Source/Version.h
    Source/Panels.h
    Source/AboutText.h
    Source/PluginProcessor.h
    Source/PluginProcessor.cpp
    Source/PluginEditor.h
    Source/PluginEditor.cpp)

target_compile_features(BeatMorph PRIVATE cxx_std_17)

target_compile_definitions(BeatMorph PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(BeatMorph
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
