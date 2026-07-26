cmake_minimum_required(VERSION 3.22)

project(FORGE VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

juce_add_plugin(FORGE
    COMPANY_NAME             "AUR Audio"
    BUNDLE_ID                com.aurveda.aur.forge
    PLUGIN_MANUFACTURER_CODE Aurv
    PLUGIN_CODE              Afg1
    FORMATS                  AU VST3 Standalone
    AU_MAIN_TYPE             kAudioUnitType_Effect
    PRODUCT_NAME             "AUR FORGE"
    IS_SYNTH                 FALSE
    NEEDS_MIDI_INPUT         FALSE
    COPY_PLUGIN_AFTER_BUILD  TRUE)

juce_generate_juce_header(FORGE)

target_sources(FORGE PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Presets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../AurvedaUI/AurLookAndFeel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../AurvedaUI/MeterComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../AurvedaUI/SpectrumAnalyzer.cpp)

target_compile_definitions(FORGE PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_include_directories(FORGE PRIVATE
    Source
    ${CMAKE_CURRENT_SOURCE_DIR}/../AurvedaDSP
    ${CMAKE_CURRENT_SOURCE_DIR}/../AurvedaUI)

target_link_libraries(FORGE PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
