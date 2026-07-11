#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
namespace ParamID{
    static constexpr auto rate = "rate";
    static constexpr auto depth = "depth";
    static constexpr auto feedback = "feedback";
    static constexpr auto mix = "mix";
    static constexpr auto bypass = "bypass";
}
namespace Params{
  inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout(){
    using namespace juce; std::vector<std::unique_ptr<RangedAudioParameter>> p;
        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{ParamID::rate,1},"Rate",
            NormalisableRange<float>(0.05f,8.0f,0.01f,0.4f),0.8f,AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){return v>=1000.f?String(v/1000.f,2)+" kHz":String(v,2)+" Hz";})));
        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{ParamID::depth,1},"Depth",
            NormalisableRange<float>(0.0f,10.0f,0.01f),4.0f,AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){return String(v,1)+" ms";})));
        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{ParamID::feedback,1},"Feedback",
            NormalisableRange<float>(0.0f,90.0f,0.1f),10.0f,AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){return String((int)v)+"%";})));
        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{ParamID::mix,1},"Mix",
            NormalisableRange<float>(0.0f,100.0f,0.1f),45.0f,AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){return String((int)v)+"%";})));
        p.push_back(std::make_unique<AudioParameterBool>(ParameterID{ParamID::bypass,1},"Bypass",false));
    return {p.begin(),p.end()};
  }
}
