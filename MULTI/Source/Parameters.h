#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
namespace ParamID{
    static constexpr auto xlo = "xlo";
    static constexpr auto xhi = "xhi";
    static constexpr auto thrL = "thrL";
    static constexpr auto ratL = "ratL";
    static constexpr auto thrM = "thrM";
    static constexpr auto ratM = "ratM";
    static constexpr auto thrH = "thrH";
    static constexpr auto ratH = "ratH";
    static constexpr auto bypass = "bypass";
}
namespace Params{
  inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout(){
    using namespace juce; std::vector<std::unique_ptr<RangedAudioParameter>> p;
        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{ParamID::xlo,1},"X-Low",
            NormalisableRange<float>(50.0f,500.0f,1.0f,0.5f),200.0f,AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){return v>=1000.f?String(v/1000.f,2)+" kHz":String(v,2)+" Hz";})));
        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{ParamID::xhi,1},"X-High",
            NormalisableRange<float>(1000.0f,8000.0f,1.0f,0.5f),2500.0f,AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){return v>=1000.f?String(v/1000.f,2)+" kHz":String(v,2)+" Hz";})));
        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{ParamID::thrL,1},"Low Thr",
            NormalisableRange<float>(-60.0f,0.0f,0.1f),-18.0f,AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){return String(v,1)+" dB";})));
        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{ParamID::ratL,1},"Low Ratio",
            NormalisableRange<float>(1.0f,10.0f,0.1f),3.0f,AudioParameterFloatAttributes()));
        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{ParamID::thrM,1},"Mid Thr",
            NormalisableRange<float>(-60.0f,0.0f,0.1f),-18.0f,AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){return String(v,1)+" dB";})));
        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{ParamID::ratM,1},"Mid Ratio",
            NormalisableRange<float>(1.0f,10.0f,0.1f),3.0f,AudioParameterFloatAttributes()));
        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{ParamID::thrH,1},"High Thr",
            NormalisableRange<float>(-60.0f,0.0f,0.1f),-18.0f,AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){return String(v,1)+" dB";})));
        p.push_back(std::make_unique<AudioParameterFloat>(ParameterID{ParamID::ratH,1},"High Ratio",
            NormalisableRange<float>(1.0f,10.0f,0.1f),3.0f,AudioParameterFloatAttributes()));
        p.push_back(std::make_unique<AudioParameterBool>(ParameterID{ParamID::bypass,1},"Bypass",false));
    return {p.begin(),p.end()};
  }
}
