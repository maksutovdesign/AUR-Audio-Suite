#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
namespace ParamID{
    static constexpr auto rate = "rate";
    static constexpr auto depth = "depth";
    static constexpr auto stereo = "stereo";
    static constexpr auto bypass = "bypass";
}
namespace Params{
  inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout(){
    using namespace juce; std::vector<std::unique_ptr<RangedAudioParameter>> p;
        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{ParamID::rate,1},"Rate",
            NormalisableRange<float>(0.1f,16.0f,0.01f,0.5f),5.0f,AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){return v>=1000.f?String(v/1000.f,2)+" kHz":String(v,2)+" Hz";})));
        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{ParamID::depth,1},"Depth",
            NormalisableRange<float>(0.0f,100.0f,0.1f),60.0f,AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){return String((int)v)+"%";})));
        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{ParamID::stereo,1},"Stereo",
            NormalisableRange<float>(0.0f,100.0f,0.1f),0.0f,AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){return String((int)v)+"%";})));
        p.push_back(std::make_unique<AudioParameterBool>(ParameterID{ParamID::bypass,1},"Bypass",false));
    return {p.begin(),p.end()};
  }
}
