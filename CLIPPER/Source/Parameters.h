#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
namespace ParamID{
    static constexpr auto drive = "drive";
    static constexpr auto ceiling = "ceiling";
    static constexpr auto mix = "mix";
    static constexpr auto soft = "soft";
    static constexpr auto bypass = "bypass";
}
namespace Params{
  inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout(){
    using namespace juce; std::vector<std::unique_ptr<RangedAudioParameter>> p;
        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{ParamID::drive,1},"Drive",
            NormalisableRange<float>(0.0f,100.0f,0.1f),30.0f,AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){return String((int)v)+"%";})));
        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{ParamID::ceiling,1},"Ceiling",
            NormalisableRange<float>(-12.0f,0.0f,0.1f),-0.3f,AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){return String(v,1)+" dB";})));
        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{ParamID::mix,1},"Mix",
            NormalisableRange<float>(0.0f,100.0f,0.1f),100.0f,AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){return String((int)v)+"%";})));
        p.push_back(std::make_unique<AudioParameterBool>(ParameterID{ParamID::soft,1},"Soft",false));
        p.push_back(std::make_unique<AudioParameterBool>(ParameterID{ParamID::bypass,1},"Bypass",false));
    return {p.begin(),p.end()};
  }
}
