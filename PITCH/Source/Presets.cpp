#include "Presets.h"
#include "Parameters.h"
namespace Presets{
  const std::vector<Preset>& getFactoryPresets(){ static const std::vector<Preset> p={
            { "Unison", { {ParamID::pitch,0.0f}, {ParamID::mix,100.0f} } },
            { "Octave Up", { {ParamID::pitch,12.0f}, {ParamID::mix,100.0f} } },
            { "Octave Down", { {ParamID::pitch,-12.0f}, {ParamID::mix,100.0f} } },
            { "Fifth", { {ParamID::pitch,7.0f}, {ParamID::mix,100.0f} } },
            { "Detune", { {ParamID::pitch,0.2f}, {ParamID::mix,50.0f} } },
  }; return p; }
  void apply(int i, juce::AudioProcessorValueTreeState& a){ const auto& p=getFactoryPresets();
    if(i<0||i>=(int)p.size())return; for(const auto&[id,v]:p[(size_t)i].values) if(auto* pr=a.getParameter(id)) pr->setValueNotifyingHost(pr->convertTo0to1(v)); }
}
