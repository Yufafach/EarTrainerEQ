cmake_minimum_required(VERSION 3.22)

project(EarTrainerEQ VERSION 1.0.0)

# Путь к JUCE, который лежит РЯДОМ с этой папкой (см. README, шаг 2-3)
add_subdirectory(../JUCE ./JUCE_build)

juce_add_plugin(EarTrainerEQ
    COMPANY_NAME "MyEarTraining"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Myco
    PLUGIN_CODE Etrn
    FORMATS VST3 Standalone
    PRODUCT_NAME "EarTrainerEQ")

juce_generate_juce_header(EarTrainerEQ)

target_sources(EarTrainerEQ
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)

target_compile_definitions(EarTrainerEQ
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(EarTrainerEQ
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

if (MSVC)
    target_compile_options(EarTrainerEQ PRIVATE /utf-8)
endif()
