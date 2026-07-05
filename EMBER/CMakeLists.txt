cmake_minimum_required(VERSION 3.22)

project(EMBER VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE via FetchContent (override with -DJUCE_PATH=/path/to/JUCE).
if(DEFINED JUCE_PATH)
    add_subdirectory(${JUCE_PATH} ${CMAKE_BINARY_DIR}/JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(EMBER
    COMPANY_NAME             "Aurveda Audio"
    BUNDLE_ID                com.aurveda.aur.ember
    PLUGIN_MANUFACTURER_CODE Aurv
    PLUGIN_CODE              Aem1
    FORMATS                  VST3 Standalone
    PRODUCT_NAME             "AUR EMBER"
    IS_SYNTH                 FALSE
    NEEDS_MIDI_INPUT         FALSE
    COPY_PLUGIN_AFTER_BUILD  TRUE)

juce_generate_juce_header(EMBER)

target_sources(EMBER PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Presets.cpp
    Source/UI/MoltenLookAndFeel.cpp
    Source/UI/MeterComponent.cpp)

target_compile_definitions(EMBER PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

# Shared AUR sound engine (header-only).
target_include_directories(EMBER PRIVATE
    Source
    ${CMAKE_CURRENT_SOURCE_DIR}/../AurvedaDSP)

target_link_libraries(EMBER PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
