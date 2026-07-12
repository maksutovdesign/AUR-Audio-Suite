#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
namespace ParamID{
    static constexpr auto attack = "attack";
    static constexpr auto sustain = "sustain";
    static constexpr auto bypass = "bypass";
}
namespace Params{
  inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout(){
    using namespace juce; std::vector<std::unique_ptr<RangedAudioParameter>> p;
        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{ParamID::attack,1},"Attack",
            NormalisableRange<float>(-100.0f,100.0f,0.1f),0.0f,AudioParameterFloatAttributes()));
        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{ParamID::sustain,1},"Sustain",
            NormalisableRange<float>(-100.0f,100.0f,0.1f),0.0f,AudioParameterFloatAttributes()));
        p.push_back(std::make_unique<AudioParameterBool>(ParameterID{ParamID::bypass,1},"Bypass",false));
    return {p.begin(),p.end()};
  }
}
