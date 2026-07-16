#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
namespace ParamID{
    static constexpr auto pitch = "pitch";
    static constexpr auto mix = "mix";
    static constexpr auto bypass = "bypass";
}
namespace Params{
  inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout(){
    using namespace juce; std::vector<std::unique_ptr<RangedAudioParameter>> p;
        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{ParamID::pitch,1},"Pitch",
            NormalisableRange<float>(-24.0f,24.0f,0.1f),0.0f,AudioParameterFloatAttributes()));
        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{ParamID::mix,1},"Mix",
            NormalisableRange<float>(0.0f,100.0f,0.1f),100.0f,AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){return String((int)v)+"%";})));
        p.push_back(std::make_unique<AudioParameterBool>(ParameterID{ParamID::bypass,1},"Bypass",false));
    return {p.begin(),p.end()};
  }
}
