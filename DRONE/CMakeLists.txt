cmake_minimum_required(VERSION 3.22)
project(DRONE VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(DEFINED JUCE_PATH)
    add_subdirectory(${JUCE_PATH} ${CMAKE_BINARY_DIR}/JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()
juce_add_plugin(DRONE
    COMPANY_NAME "AUR Audio" BUNDLE_ID com.aurveda.aur.drone
    PLUGIN_MANUFACTURER_CODE Aurv PLUGIN_CODE Adr1
    FORMATS AU VST3 Standalone
    IS_SYNTH TRUE NEEDS_MIDI_INPUT TRUE
    AU_MAIN_TYPE kAudioUnitType_MusicDevice
    PRODUCT_NAME "AUR DRONE" COPY_PLUGIN_AFTER_BUILD TRUE)
juce_generate_juce_header(DRONE)
target_sources(DRONE PRIVATE
    Source/PluginProcessor.cpp Source/PluginEditor.cpp Source/Presets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../AurvedaUI/AurLookAndFeel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../AurvedaUI/MeterComponent.cpp)
target_compile_definitions(DRONE PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_include_directories(DRONE PRIVATE Source
    ${CMAKE_CURRENT_SOURCE_DIR}/../AurvedaDSP
    ${CMAKE_CURRENT_SOURCE_DIR}/../AurvedaSynth
    ${CMAKE_CURRENT_SOURCE_DIR}/../AurvedaUI)
target_link_libraries(DRONE PRIVATE juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
