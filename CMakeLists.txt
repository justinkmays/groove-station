cmake_minimum_required(VERSION 3.22)

project(GrooveStation VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE as a subdirectory
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../JUCE ${CMAKE_BINARY_DIR}/JUCE)

# Define the plugin/standalone target
juce_add_plugin(GrooveStation
    COMPANY_NAME "GrooveStation"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE GrvS
    PLUGIN_CODE GrSt
    FORMATS Standalone VST3 AU
    PRODUCT_NAME "Groove Station"
    BUNDLE_ID "com.groovestation.sampler"
)

juce_generate_juce_header(GrooveStation)

target_sources(GrooveStation
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/Audio/SampleVoice.cpp
        Source/Audio/SamplerEngine.cpp
        Source/Audio/ADSREnvelope.cpp
        Source/Audio/EffectsChain.cpp
        Source/Sequencer/StepSequencer.cpp
        Source/Sequencer/Pattern.cpp
        Source/UI/PadGrid.cpp
        Source/UI/WaveformDisplay.cpp
        Source/UI/TransportBar.cpp
        Source/UI/MixerPanel.cpp
        Source/UI/EffectsPanel.cpp
        Source/UI/SequencerGrid.cpp
        Source/UI/CustomLookAndFeel.cpp
)

target_include_directories(GrooveStation
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_compile_definitions(GrooveStation
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(GrooveStation
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
