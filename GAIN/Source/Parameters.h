#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
namespace ParamID{
    static constexpr auto gain = "gain";
    static constexpr auto balance = "balance";
    static constexpr auto width = "width";
    static constexpr auto mono = "mono";
    static constexpr auto phase = "phase";
    static constexpr auto bypass = "bypass";
}
namespace Params{
  inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout(){
    using namespace juce; std::vector<std::unique_ptr<RangedAudioParameter>> p;
        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{ParamID::gain,1},"Gain",
            NormalisableRange<float>(-24.0f,24.0f,0.01f),0.0f,AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){return String(v,1)+" dB";})));
        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{ParamID::balance,1},"Balance",
            NormalisableRange<float>(-1.0f,1.0f,0.01f),0.0f,AudioParameterFloatAttributes()));
        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{ParamID::width,1},"Width",
            NormalisableRange<float>(0.0f,200.0f,1.0f),100.0f,AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){return String((int)v)+"%";})));
        p.push_back(std::make_unique<AudioParameterBool>(ParameterID{ParamID::mono,1},"Mono",false));
        p.push_back(std::make_unique<AudioParameterBool>(ParameterID{ParamID::phase,1},"Phase",false));
        p.push_back(std::make_unique<AudioParameterBool>(ParameterID{ParamID::bypass,1},"Bypass",false));
    return {p.begin(),p.end()};
  }
}
